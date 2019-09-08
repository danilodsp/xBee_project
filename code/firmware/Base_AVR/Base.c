/*
Programa para MCU Base do "Projeto para Automa��o usando xBee's"
MCU: AtMega328P
*/

// ******************** Bibliotecas ********************
#include <avr/io.h>
#include <avr/common.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>

// ******************** Clock usado para Delay ********************
#define F_CPU 1000000UL  // 1 MHz

// ******************** Defini��es ********************
#define BAUD 103 // Baud rate
#define LED1 0x08 // Envia dados para PC
#define LED2 0x10 // Recebe dados do xBee (apaga quando manda para PC)
#define LED3 0x20 // Desliga UART ou Envia dados para xBee
#define LED4 0x40 // Erro de UART

// ******************** Vari�veis globais ********************
volatile char resposta; // Var tempor�ria para recep��o de UDR0(UART)
volatile int cont; // Tamanho do frame usado para enviar dados constantes para PC
volatile unsigned int contador; // Contador do Timer0
// EEPROM
volatile int addr; // Endere�o vari�vel da tabela 2 da EEPROM
volatile int addr2; // Endere�o vari�vel da tabela 1 da EEPROM
volatile int totalReg; // Total de registros da tabela 2 da EEPROM
volatile int totalRegTab1; // Total de registros da tabela 1 da EEPROM
// Frames
volatile int frame[7]; // Frame de comando para xBee
volatile int frameEeprom[11]; // Frame para armazenar na EEPROM
volatile int frameAtual[40]; // Frame recebido fica na RAM
volatile int sensorEeprom[12]; // Frame para armazenamento da tabela 1
// Flags
volatile int flagEeprom; // 0- N�o precisa armazenar na EEPROM; 1- Precisa
volatile int flagTab1; // 1- Precisa armazenar na tabela 1
volatile int tipoEnvio; // 0- uC pata PC; 1- uC para xBee
volatile int flagTx; // 1- Entra em txbyte() e envia dados (TX)
// Estados
volatile int comand; // Estados de recebimento de dados (RX)
volatile int estadoEeprom; // Estados da EEPROM
// Dados de frameAtual
volatile unsigned long int checksum; // Checksum do frameAtual
volatile int tam0; // Tamanho L
volatile int tam1; // Tamanho H
volatile int i; // Cotrole de posi��o do frameAtual
// Hora e Data
volatile unsigned int hora[3]; // seg, min e hora
volatile unsigned int data[3]; // dia, mes e ano
int TEMP1; // Recarrego com o necess�rio que falta para 1seg em 16bits(16MHz)
int TEMP2; // Parte L de TEMP

// ******************** MAIN ********************
int main(void){
		// ******************** Inicializa��o e configura��es ********************
        DDRB = (1<<DDB3)|(1<<DDB4)|(1<<DDB5)|(1<<DDB6)|(1<<DDB7); // Pinos de sa�da
        SREG = 1<<SREG_I; // Interrpt Enable geral
        //CLKPR |= 1<<CLKPCE;
        //CLKPR |= 0x04;

		// Inicializa��o de vari�veis
        cont = 8; // Tamanho do frame constante de dados para o PC
        addr = 0;
		addr2 = 0;
        tipoEnvio = 0; // 0- Envia frame para UART_TX ;1- Envia dados da EEPROM para UART_TX
        flagEeprom = 0;
        contador = 0; // contador para Timer
        tam0 = 0;
        tam1 = 0;
        comand = 0;
        checksum = 0;
		flagTx = 0;
        TEMP1 = 0xC2;
        TEMP2 = 0xF6;

		// Inicializa��o de data e hora
        data[0]=10; // Dia
        data[1]=07; // M�s
        data[2]=11; // Ano
        hora[0]=55; // Segundo
        hora[1]=29; // Minuto
        hora[2]=19; // Hora

        // Frame constante de dados para o PC
		frame[0] = 0x7E; //7E
        frame[1] = 0x00; //00
        frame[2] = 0x04; //04
        frame[3] = 0x08; //08
        frame[4] = 0x52; //52
        frame[5] = 0x4E; //4E
        frame[6] = 0x4A; //4A
        frame[7] = 0x0D; //0D

        // Configura��o de UART0
        UCSR0A &= ~(1<<TXC0); // �ltima transmiss�o completa
		//UCSR0A &= ~(1<<UDRE0); // Buffer vazio
        UCSR0B = (1<<RXEN0)|(1<<TXEN0); // Habilita o RX e TX
        UCSR0C = 0x06; // 8bits de transmiss�o
        UBRR0L = BAUD; // Set Baud Rate
        UBRR0H = (BAUD>>8);
        UCSR0B |= (1<<RXCIE0); // Habilita recep��o

        // Configura��o de Timer0
        TIMSK0 &= ~(1<<TOIE0);
        TCCR0B = 0x05; // Prescaller 1024

        // Configura��o de Timer1
    	TIMSK1 |= (1<<TOIE1);
    	TCCR1B = 0x05; // Prescaller 1024

        // Configura��o de EEPROM
        //uint8_t ByteOfData;
		totalReg = eeprom_read_byte((uint8_t*)(101)); // Carrega totalReg
		totalRegTab1 = eeprom_read_byte((uint8_t*)(1)); // Carrega totalRegTab1

		/*_delay_ms(1000);
		eeprom_write_byte((uint8_t*)1,1);
		_delay_ms(1000);
		eeprom_write_byte((uint8_t*)101,1);*/

        // ******************** Loop infinito ********************
        while(1){
                /*if((UCSR0A&FE0)|(UCSR0A&DOR0)|(UCSR0A&UPE0)){ // Tratamento de erro de UART
                    PORTB |= LED4;
                }*/
                if(flagEeprom==1){
                    TIMSK0 |= (1<<TXEN0)|(1<<TOIE0); // Liga o Timer
                }
				if((flagTx)&(UCSR0A&(1<<UDRE0))){ // flagTx e Buffer vazio
					txbyte(); // Para enviar dados
				}
        };
}

// ******************** Envio de dados (TX) ********************
void txbyte(){
	if(addr2==0){
		// Enviando tabela 2
		UDR0 = eeprom_read_byte((uint8_t*)(totalReg - addr + 100)); // Envia dados da EEPROM
		if(addr==0){ // Acabou
			flagTx=0; // Desliga o txbyte()
        	totalReg=1; // Valor default
			eeprom_write_byte((uint8_t*)(101),totalReg); // Armazena o valor default
    	}
    	else{
    		addr--; // Decrementa, para incrementar (diferen�a totalReg-addr)
    	}	
	}
	else{
		// Enviando tabela 1
		UDR0 = eeprom_read_byte((uint8_t*)(totalRegTab1 - addr2+1)); // Envia dados da EEPROM
		addr2--; // Decrementa, para incrementar (diferen�a totalRegTab1-addr2)
	}

	
}

// Interrup��o UART RX
ISR(USART_RX_vect){
        resposta = UDR0; // Recebe do buffer e armazena em uma vari�vel global
        
        // Estado 0
        if(comand==0){
                if(resposta==0x7E){ // Recebe dados do xBee
                comand = 1; // Pr�ximo estado
                }
                else if(resposta==0x01){ // Recebe comando do PC
                        comand = 4; // Pr�ximo estado
                }
        }
        
        // Estado 1
        else if(comand==1){
                tam1=resposta; // Primeiro byte do tamanho, MSB
                checksum = 0; // Zerando checksum
                comand = 2; // Pr�ximo estado
                
        }

        // Estado 2
        else if(comand==2){
                tam0 = resposta; // Segundo byte do tamanho, LSB
                frameAtual[0] = 0x7E; // Armazena os primeiros bytes j� conhecidos
                frameAtual[1] = tam1; // ...
                frameAtual[2] = tam0; // ...
                
                i=3; // Quantidade de byte conhecidos, 0x7E, tam1 e tam0
                comand = 3; // Pr�ximo estado
        }

        // Estado 3
        else if(comand==3){
                frameAtual[i] = resposta; // Armazena os bytes do xBee
                checksum += resposta; // Incrementa checksum

                if(i==(tam0+3)){ // Verifica se chegou ao fim do frame
                        
                        if(checksum&0xFF){ // Verifica se o checksum est� correto
//**************Temos o frame****************************************************

//**************Caso seja um frame de sample*************************************

if(frameAtual[3]==0x92){
	
	sensorEeprom[0]='$'; // Alguma coisa da cabe�a de Besch
	sensorEeprom[1]='A'; // ID H
	sensorEeprom[2]='0'; // ID L
	sensorEeprom[3]=frameAtual[4]; // Endere�o do xBee
	sensorEeprom[4]=frameAtual[5];
	sensorEeprom[5]=frameAtual[6];
	sensorEeprom[6]=frameAtual[7];
	sensorEeprom[7]=frameAtual[8];
	sensorEeprom[8]=frameAtual[9];
	sensorEeprom[9]=frameAtual[10];
	sensorEeprom[10]=frameAtual[11];
	sensorEeprom[11]='\n'; // 0x0A

	if(existeSerial()==0){ // Verifica se existe outro ID igual na EEPROM
		totalRegTab1+=12; // Incrementa a contagem de 12 bytes
		flagTab1 = 1; // Habilita o armazenamento da EEPROM na tabela 1
	}



    frameEeprom[0]=data[0]; // Dia
    frameEeprom[1]=data[1]; // M�s
    frameEeprom[2]=data[2]; // Ano
    frameEeprom[3]=hora[2]; // Hora
    frameEeprom[4]=hora[1]; // Minuto
    frameEeprom[5]=hora[0]; // Segundo
    frameEeprom[6]='A'; // ID_H
    frameEeprom[7]='0'; // ID_L
    frameEeprom[8]=frameAtual[tam0+1]; // Armazena os dados na EEPROM
    frameEeprom[9]=frameAtual[tam0+2]; // Armazena os dados na EEPROM
    frameEeprom[10]='\n'; // Quebra de linha

    totalReg+=11; // Incrementa contagem de 11 bytes
    flagEeprom=1; // Armazena na EEPROM
    estadoEeprom=0;   
        







}

//**************Fim do tratamento de sample**************************************

//**************Fim do tratamento de frame***************************************
                        }
                        comand=0; // Pr�ximo estado
                }        
                i++;        
        }

        // Estado 4
        else if(comand==4){
                if(resposta==0xFE){
//***************Tratamento de requisi��o de dados*******************************                        

addr = totalReg;
addr--; // Para come�ar com o endere�o da eeprom 1

addr2 = totalRegTab1;
//addr2--;

flagTx=0x20;
UCSR0B |= (1<<TXEN0)|(1<<TXCIE0);
//PORTB|=LED1;



//**************Fim do tratamento de dados***************************************
                }
                comand = 0; // Pr�ximo estado
        }
}

int existeSerial(){
	int j;
	int total;
	int serialEeprom[8]; // serial lido da eeprom
	int serialAtual[8]; // serial do atual frame recebido

	serialAtual[0] = sensorEeprom[3];
	serialAtual[1] = sensorEeprom[4];
	serialAtual[2] = sensorEeprom[5];
	serialAtual[3] = sensorEeprom[6];
	serialAtual[4] = sensorEeprom[7];
	serialAtual[5] = sensorEeprom[8];
	serialAtual[6] = sensorEeprom[9];
	serialAtual[7] = sensorEeprom[10];

	total = eeprom_read_byte((uint8_t*)(1)); // totalRegTab1 tabela 1
	j = 5;

	while(j<total){
		serialEeprom[0] = eeprom_read_byte((uint8_t*)(j));
		j++;
		serialEeprom[1] = eeprom_read_byte((uint8_t*)(j));
		j++;
		serialEeprom[2] = eeprom_read_byte((uint8_t*)(j));
		j++;
		serialEeprom[3] = eeprom_read_byte((uint8_t*)(j));
		j++;
		serialEeprom[4] = eeprom_read_byte((uint8_t*)(j));
		j++;
		serialEeprom[5] = eeprom_read_byte((uint8_t*)(j));
		j++;
		serialEeprom[6] = eeprom_read_byte((uint8_t*)(j));
		j++;
		serialEeprom[7] = eeprom_read_byte((uint8_t*)(j));
		j++;
		

		if((serialAtual[7]==serialEeprom[7])&
		(serialAtual[6]==serialEeprom[6])&
		(serialAtual[5]==serialEeprom[5])&
		(serialAtual[4]==serialEeprom[4])&
		(serialAtual[3]==serialEeprom[3])&
		(serialAtual[2]==serialEeprom[2])&
		(serialAtual[1]==serialEeprom[1])&
		(serialAtual[0]==serialEeprom[0])){
			PORTB|=LED1;
			return 1;
		}

		j+=4;
	}
	
	return 0;
}

// Armazena dados na EEPROM
void save_eeprom(){
        if(estadoEeprom==0){
                eeprom_write_byte((uint8_t*)(totalReg+100),frameEeprom[10]);
                estadoEeprom=1;
        }
        else if(estadoEeprom==1){
                eeprom_write_byte((uint8_t*)(totalReg+99),frameEeprom[9]);
                estadoEeprom=2;
        }
        else if(estadoEeprom==2){
                eeprom_write_byte((uint8_t*)(totalReg+98),frameEeprom[8]);
                estadoEeprom=3;
        }
        else if(estadoEeprom==3){
                eeprom_write_byte((uint8_t*)(totalReg+97),frameEeprom[7]);
                estadoEeprom=4;
        }
        else if(estadoEeprom==4){
                eeprom_write_byte((uint8_t*)(totalReg+96),frameEeprom[6]);
                estadoEeprom=5;
        }
        else if(estadoEeprom==5){
                eeprom_write_byte((uint8_t*)(totalReg+95),frameEeprom[5]);
                estadoEeprom=6;
        }
        else if(estadoEeprom==6){
                eeprom_write_byte((uint8_t*)(totalReg+94),frameEeprom[4]);
                estadoEeprom=7;
        }
        else if(estadoEeprom==7){
                eeprom_write_byte((uint8_t*)(totalReg+93),frameEeprom[3]);
                estadoEeprom=8;
        }
        else if(estadoEeprom==8){
                eeprom_write_byte((uint8_t*)(totalReg+92),frameEeprom[2]);
                estadoEeprom=9;
        }
        else if(estadoEeprom==9){
                eeprom_write_byte((uint8_t*)(totalReg+91),frameEeprom[1]);
                estadoEeprom=10;
        }
        else if(estadoEeprom==10){
                eeprom_write_byte((uint8_t*)(totalReg+90),frameEeprom[0]);
                estadoEeprom=11;
        }
        else if(estadoEeprom==11){
                eeprom_write_byte((uint8_t*)(101),totalReg);
                //PORTB|=LED1;
                if(flagTab1==1){
					estadoEeprom=12;
				}
				else{
					estadoEeprom=0;
					flagEeprom = 0; // Acabou de armazenar na EEPROM
					TIMSK0 &= ~(1<<TOIE0); // Desliga o Timer
				}
        }
		else if(estadoEeprom==12){
                eeprom_write_byte((uint8_t*)(totalRegTab1),sensorEeprom[11]);
                estadoEeprom=13;
        }
		else if(estadoEeprom==13){
                eeprom_write_byte((uint8_t*)(totalRegTab1-1),sensorEeprom[10]);
                estadoEeprom=14;
        }
		else if(estadoEeprom==14){
                eeprom_write_byte((uint8_t*)(totalRegTab1-2),sensorEeprom[9]);
                estadoEeprom=15;
        }
		else if(estadoEeprom==15){
                eeprom_write_byte((uint8_t*)(totalRegTab1-3),sensorEeprom[8]);
                estadoEeprom=16;
        }
		else if(estadoEeprom==16){
                eeprom_write_byte((uint8_t*)(totalRegTab1-4),sensorEeprom[7]);
                estadoEeprom=17;
        }
		else if(estadoEeprom==17){
                eeprom_write_byte((uint8_t*)(totalRegTab1-5),sensorEeprom[6]);
                estadoEeprom=18;
        }
		else if(estadoEeprom==18){
                eeprom_write_byte((uint8_t*)(totalRegTab1-6),sensorEeprom[5]);
                estadoEeprom=19;
        }
		else if(estadoEeprom==19){
                eeprom_write_byte((uint8_t*)(totalRegTab1-7),sensorEeprom[4]);
                estadoEeprom=20;
        }
		else if(estadoEeprom==20){
                eeprom_write_byte((uint8_t*)(totalRegTab1-8),sensorEeprom[3]);
                estadoEeprom=21;
        }
		else if(estadoEeprom==21){
                eeprom_write_byte((uint8_t*)(totalRegTab1-9),sensorEeprom[2]);
                estadoEeprom=22;
        }
		else if(estadoEeprom==22){
                eeprom_write_byte((uint8_t*)(totalRegTab1-10),sensorEeprom[1]);
                estadoEeprom=23;
        }
		else if(estadoEeprom==23){
                eeprom_write_byte((uint8_t*)(totalRegTab1-11),sensorEeprom[0]);
                estadoEeprom=24;
        }
		else if(estadoEeprom==24){
                eeprom_write_byte((uint8_t*)(1),totalRegTab1);
                estadoEeprom=0;
				flagEeprom = 0; // Acabou de armazenar na EEPROM
				flagTab1 = 0; // Acabou de armazenar na tabela 1 da EEPROM
				TIMSK0 &= ~(1<<TOIE0); // Desliga o Timer
        }
}

// Interrup��o de Timer0
ISR(TIMER0_OVF_vect){
        if(contador<1){ // Contagem do timer
                contador++;
        }
        else{
                save_eeprom();
                contador = 0;
        }
}

ISR(TIMER1_OVF_vect){
        TCNT1H = TEMP1;
        TCNT1L = TEMP2;
        if(hora[0]<59)
                hora[0]++; // seg
        else{
                hora[0]=0;
                if(hora[1]<59)
                        hora[1]++; // min
                else{
                        hora[1]=0;
                        if(hora[2]<59)
                                hora[2]++; // hora
                        else{
                                hora[2]=0;
                                if(data[0]<31)
                                        data[0]++; // dia
                                else{
                                        data[0]=0;
                                        if(data[1]<12)
                                                data[1]++; // mes
                                        else{
                                                data[1]=0;
                                                data[2]++; // ano
                                        }
                                }
                        }
                }
        }
}
