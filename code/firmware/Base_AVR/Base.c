/*
Programa para MCU Base do "Projeto para Automação usando xBee's"
MCU: AtMega328P
*/
#include <avr/io.h>
#include <avr/common.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#define F_CPU 1000000UL  // 1 MHz

#define BAUD 103
#define LED1 0x08 // Envia dados para PC
#define LED2 0x10 // Recebe dados do xBee (apaga quando manda para PC)
#define LED3 0x20 // Desliga UART ou Envia dados para xBee
#define LED4 0x40 // Erro de UART
/*
FUSES =
{
        .low = LFUSE_DEFAULT,
        .high = HFUSE_DEFAULT,
        .extended = EFUSE_DEFAULT,
};*/

volatile int cont; // Tamanho do frame
volatile char resposta; // Var temporária para recepção de UDR0(UART)
volatile int addr; // Endereço de EEPROM
volatile int totalReg; // Total de registro da EEPROM
volatile int frame[7]; // frame de comando para xBee
volatile int frameEeprom[40]; // frame para armazenar na EEPROM
volatile int frameAtual[40]; // frame recebido
volatile int flagEeprom; // 0- Não precisa armazenar na EEPROM; 1- Precisa
volatile int tipoEnvio; // 0- uC pata PC; 1- uC para xBee
volatile int comand; // Estados
volatile int estadoEeprom; // Estados da EEPROM
volatile unsigned long int checksum;
volatile int tam0;
volatile int tam1;
volatile int i;
volatile unsigned int contador; // Contador do Timer0
// Hora e Data
volatile unsigned int hora[3]; // seg, min e hora
volatile unsigned int data[3]; // dia, mes e ano
int TEMP1; // Recarrego com o necessário que falta para 1seg em 16bits(16MHz)
int TEMP2;
volatile int flagTx;
volatile unsigned int aEnviar;

int main(void){
        DDRB = (1<<DDB3)|(1<<DDB4)|(1<<DDB5)|(1<<DDB6)|(1<<DDB7); // Saída
        SREG = 1<<SREG_I;
        //CLKPR |= 1<<CLKPCE;
        //CLKPR |= 0x04;
        cont = 8;
        addr = 0;
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

        data[0]=10; // Dia
        data[1]=07; // Mês
        data[2]=11; // Ano
        hora[0]=55; // Segundo
        hora[1]=29; // Minuto
        hora[2]=19; // Hora

        frame[0] = 0x7E; //7E
        frame[1] = 0x00; //00
        frame[2] = 0x04; //04
        frame[3] = 0x08; //08
        frame[4] = 0x52; //52
        frame[5] = 0x4E; //4E
        frame[6] = 0x4A; //4A
        frame[7] = 0x0D; //0D

        // Button
        PCICR = 1<<PCIE0; // Habilitando interrupção do PORTB
        PCMSK0 = (1<<PCINT0)|(1<<PCINT1)|(1<<PCINT2); // Definindo pinos de interrupção PCINT1 (B0)

        // UART0
        UCSR0A &= ~(1<<TXC0); // Última transmissão completa
		//UCSR0A &= ~(1<<UDRE0); // Buffer vazio
        UCSR0B = (1<<RXEN0)|(1<<TXEN0); // Habilita o RX e TX
        UCSR0C = 0x06; // 8bits de transmissão
        UBRR0L = BAUD; // Set Baud Rate
        UBRR0H = (BAUD>>8);
        UCSR0B |= (1<<RXCIE0); // Habilita recepção

        // Timer0
        TIMSK0 &= ~(1<<TOIE0);
        TCCR0B = 0x05; // Prescaller 1024

        // Timer1
    	TIMSK1 |= (1<<TOIE1);
    	TCCR1B = 0x05; // Prescaller 1024

        // EEPROM
        //uint8_t ByteOfData;
		totalReg = eeprom_read_byte((uint8_t*)(1));

        //UCSR0B |= (1<<TXEN0)|(1<<TXCIE0);
		//flagTx = 0xFF;

        // Loop
        while(1){
                /*if((UCSR0A&FE0)|(UCSR0A&DOR0)|(UCSR0A&UPE0)){ // Tratamento de erro de UART
                    PORTB |= LED4;
                }*/
                if(flagEeprom==1){
                    TIMSK0 |= (1<<TXEN0)|(1<<TOIE0); // Liga o Timer
                }
				if((flagTx)&(UCSR0A&(1<<UDRE0))){// &~(UCSR0A|~(1<<TXC0))
					txbyte();
				}
        };
}
/*
// Interrupção externa
ISR(PCINT0_vect){
        if(PINB&0x01){ // Button1 Envia dados para PC
                if(addr>0){
                        UCSR0B |= (1<<TXEN0)|(1<<TXCIE0);
                        PORTB |= LED1;
                        PORTB &= ~LED3;
                        tipoEnvio = 0;
                }
        }
        else if(PINB&0x02){ // Button2 Desliga a UART
                UCSR0B = 0x00;
                PORTB |= LED3;
        }
        else if(PINB&0x04){ // Button3 Envia dados para xBee
                tipoEnvio = 1;
                cont=0;
                UCSR0B |= (1<<TXEN0)|(1<<TXCIE0);
                PORTB |= LED3;
        }
}*/
/*
// Interrupção UART TX
ISR(USART_TX_vect){
        if(tipoEnvio == 0){
                UDR0 = eeprom_read_byte((uint8_t*)(totalReg - addr));
				PORTB|=LED1;
                //UDR0 = frameEeprom[totalReg-addr];
                if(addr==0){
                        UCSR0B &= ~(1<<TXCIE0);
                        totalReg=0;
                }
                else{
                        addr--;
                }
        }
        else if(tipoEnvio == 1){
                UDR0 = frame[cont]; // Envia frame da flash
                cont++;
                if(cont>7){
                        UCSR0B &= ~(1<<TXCIE0);
                }
        }
}*/

void txbyte(){
	//UCSR0A &= ~(1<<TXC0);
	UDR0 = eeprom_read_byte((uint8_t*)(totalReg - addr));
    //UDR0 = frameEeprom[totalReg-addr];
    if(addr==0){
    	//UCSR0B &= ~(1<<TXCIE0);
		flagTx=0;
        totalReg=1;
		eeprom_write_byte((uint8_t*)(1),totalReg);
    }
    else{
    	addr--;
    }
}

// Interrupção UART RX
ISR(USART_RX_vect){
        resposta = UDR0;
        

        // Estado 0
        if(comand==0){
                if(resposta==0x7E){ // Recebe dados do xBee
                comand = 1; // Próximo estado
                //PORTB &= ~LED1;
                }
                else if(resposta==0x01){ // Recebe comando do PC
                        comand = 4;
                }
        }
        
        // Estado 1
        else if(comand==1){
                tam1=resposta; // Primeiro byte do tamanho, MSB
                checksum = 0;
                comand = 2; // Próximo estado
                
        }

        // Estado 2
        else if(comand==2){
                tam0 = resposta; // Segundo byte do tamanho, LSB
                frameAtual[0] = 0x7E; // Armazena os primeiros bytes já conhecidos
                frameAtual[1] = tam1;
                frameAtual[2] = tam0;
                
                i=3;
                comand = 3; // Próximo estado
        }

        // Estado 3
        else if(comand==3){
                frameAtual[i] = resposta; // Armazena os bytes do xBee
                checksum += resposta; // Incrementa checksum

                if(i==(tam0+3)){
                        
                        if(checksum&0xFF){
//**************Temos o frame****************************************************

//**************Caso seja um frame de sample*************************************

if(frameAtual[3]==0x92){
        addr=totalReg;

		addr++;
        frameEeprom[addr]=data[0]; // Dia
        
        addr++;
        frameEeprom[addr]=data[1]; // Mês

        addr++;
        frameEeprom[addr]=data[2]; // Ano

        addr++;
        frameEeprom[addr]=hora[2]; // Hora

        addr++;
        frameEeprom[addr]=hora[1]; // Minuto

        addr++;
        frameEeprom[addr]=hora[0]; // Segundo

        /*
        addr++;
        frameEeprom[addr]=26; // Dia
        
        addr++;
        frameEeprom[addr]=05; // Mês

        addr++;
        frameEeprom[addr]=11; // Ano

        addr++;
        frameEeprom[addr]=15; // Hora

        addr++;
        frameEeprom[addr]=36; // Minuto

        addr++;
        frameEeprom[addr]=21; // Segundo*/

        addr++;
        frameEeprom[addr]='A'; // ID_H

        addr++;
        frameEeprom[addr]='0'; // ID_L

        addr++;
        frameEeprom[addr]=frameAtual[tam0+1]; // Armazena os dados na EEPROM

        addr++;
        frameEeprom[addr]=frameAtual[tam0+2]; // Armazena os dados na EEPROM

        addr++;
        frameEeprom[addr]='\n'; // Quebra de linha

        totalReg=addr;
        frameEeprom[1]=totalReg;
        flagEeprom=1; // Armazena na EEPROM
        estadoEeprom=0;

		//PORTB|=LED1;
        
        //PORTB &= ~LED1;
        /*
        if(addr==11){
			PORTB|=LED1;
		}*/
        
        







}

//**************Fim do tratamento de sample**************************************

//**************Fim do tratamento de frame***************************************
                        }
                        comand=0; // Próximo estado
                }        
                i++;        
        }

        // Estado 4
        else if(comand==4){
                if(resposta==0xFE){
//***************Tratamento de requisição de dados*******************************                        

addr = totalReg;
addr--; // Para começar com o endereço da eeprom 1

flagTx=0x20;
UCSR0B |= (1<<TXEN0)|(1<<TXCIE0);
//PORTB|=LED1;





//**************Fim do tratamento de dados***************************************
                }
                comand = 0; // Próximo estado
        }
}

// Armazena dados na EEPROM
void save_eeprom(){
        if(estadoEeprom==0){
                eeprom_write_byte((uint8_t*)(totalReg),frameEeprom[totalReg]);
                estadoEeprom=1;
        }
        else if(estadoEeprom==1){
                eeprom_write_byte((uint8_t*)(totalReg-1),frameEeprom[totalReg-1]);
                estadoEeprom=2;
        }
        else if(estadoEeprom==2){
                eeprom_write_byte((uint8_t*)(totalReg-2),frameEeprom[totalReg-2]);
                estadoEeprom=3;
        }
        else if(estadoEeprom==3){
                eeprom_write_byte((uint8_t*)(totalReg-3),frameEeprom[totalReg-3]);
                estadoEeprom=4;
        }
        else if(estadoEeprom==4){
                eeprom_write_byte((uint8_t*)(totalReg-4),frameEeprom[totalReg-4]);
                estadoEeprom=5;
        }
        else if(estadoEeprom==5){
                eeprom_write_byte((uint8_t*)(totalReg-5),frameEeprom[totalReg-5]);
                estadoEeprom=6;
        }
        else if(estadoEeprom==6){
                eeprom_write_byte((uint8_t*)(totalReg-6),frameEeprom[totalReg-6]);
                estadoEeprom=7;
        }
        else if(estadoEeprom==7){
                eeprom_write_byte((uint8_t*)(totalReg-7),frameEeprom[totalReg-7]);
                estadoEeprom=8;
        }
        else if(estadoEeprom==8){
                eeprom_write_byte((uint8_t*)(totalReg-8),frameEeprom[totalReg-8]);
                estadoEeprom=9;
        }
        else if(estadoEeprom==9){
                eeprom_write_byte((uint8_t*)(totalReg-9),frameEeprom[totalReg-9]);
                estadoEeprom=10;
        }
        else if(estadoEeprom==10){
                eeprom_write_byte((uint8_t*)(totalReg-10),frameEeprom[totalReg-10]);
                estadoEeprom=11;
        }
        else if(estadoEeprom==11){
                eeprom_write_byte((uint8_t*)(1),frameEeprom[1]);
				if(frameEeprom[1]==12){
					PORTB |= LED1;
				}
                TIMSK0 &= ~(1<<TOIE0); // Desliga o Timer
                addr = totalReg;
                flagEeprom = 0; // Acabou de armazenar na EEPROM
                //PORTB|=LED1;
                estadoEeprom=0;
        }
}

// Interrupção de Timer0
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
