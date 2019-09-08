/*
Programa para MCU Base do "Projeto para Automação usando xBee's"
MCU: AtMega328P
*/
#include <avr/io.h>
#include <avr/common.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>

//#define F_CPU 1000000UL  // 1 MHz

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
volatile int flagEeprom; // 0- Não precisa armazenar na EEPROM; 1- Precisa
volatile int tipoEnvio; // 0- uC pata PC; 1- uC para xBee
volatile unsigned int contador; // Contador do Timer0

int main(void){
	DDRB = (1<<DDB3)|(1<<DDB4)|(1<<DDB5)|(1<<DDB6)|(1<<DDB7); // Saída
	SREG = 1<<SREG_I;
	//CLKPR |= 1<<CLKPCE;
	//CLKPR = 0x04;
	cont = 0;

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
	UCSR0B = (1<<RXEN0)|(1<<TXEN0); // Habilita o RX e TX
	UCSR0C = 0x06; // 8bits de transmissão
	UBRR0L = BAUD; // Set Baud Rate
	UBRR0H = (BAUD>>8);
	UCSR0B |= (1<<RXCIE0); // Habilita recepção

	// Loop
	while(1);
}

// Interrupção externa
ISR(PCINT0_vect){
	if(PINB&0x04){ // Button3 Envia dados para xBee
		cont=0;
		UCSR0B |= (1<<TXEN0)|(1<<TXCIE0);
		PORTB |= LED3;
	}
}

// Interrupção UART TX
ISR(USART_TX_vect){
	UDR0 = frame[cont];
	cont++;
	if(cont>7){
		UCSR0B &= ~(1<<TXCIE0);
	}
}
