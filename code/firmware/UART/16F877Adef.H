// Definições de endereços, PIC16F877A

// endereços USART
#byte TXSTA = 0x98
#byte RCSTA = 0x18
#byte SPBRG = 0x99
#byte TXREG = 0x19
#byte RCREG = 0x1A

// endereços gerais
#byte INTCON	= 0x0B
#byte PIR1		= 0x0C
#byte PIE1		= 0x8C
#byte OPTIONREG	= 0x81

// endereços para PORTB
#byte TRISA	= 0x85
#byte TRISB	= 0x86
#byte TRISC	= 0x87
#byte PORTA	= 0x05
#byte PORTB	= 0x06
#byte PORTC	= 0x07

// definição dos bits
#bit CSRC	= TXSTA.7
#bit TX9	= TXSTA.6
#bit TXEN	= TXSTA.5
#bit SYNC	= TXSTA.4
#bit BRGH	= TXSTA.2
#bit TRMT	= TXSTA.1
#bit TX9D	= TXSTA.0

#bit SPEN	= RCSTA.7
#bit RX9	= RCSTA.6
#bit SREN	= RCSTA.5
#bit CREN	= RCSTA.4
#bit ADDEN	= RCSTA.3
#bit FERR	= RCSTA.2
#bit OERR	= RCSTA.1
#bit RX9D	= RCSTA.0

#bit GIE	= INTCON.7
#bit PEIE	= INTCON.6
#bit INTE	= INTCON.4
#bit RBIE	= INTCON.3

#bit RCIF	= PIR1.5
#bit TXIF	= PIR1.4

#bit ADIE	= PIE1.6
#bit RCIE	= PIE1.5
#bit TXIE	= PIE1.4

#bit TRISB_0	= TRISB.0
#bit TRISB_1	= TRISB.1
#bit TRISB_2	= TRISB.2
#bit TRISB_3	= TRISB.3
#bit TRISB_4	= TRISB.4
#bit TRISB_5	= TRISB.5
#bit TRISB_6	= TRISB.6
#bit TRISB_7	= TRISB.7

#bit PORTA_0	= PORTA.0
#bit PORTA_1	= PORTA.1
#bit PORTA_2	= PORTA.2
#bit PORTA_3	= PORTA.3
#bit PORTA_4	= PORTA.4
#bit PORTA_5	= PORTA.5
#bit PORTA_6	= PORTA.6
#bit PORTA_7	= PORTA.7

#bit PORTB_0	= PORTB.0
#bit PORTB_1	= PORTB.1
#bit PORTB_2	= PORTB.2
#bit PORTB_3	= PORTB.3
#bit PORTB_4	= PORTB.4
#bit PORTB_5	= PORTB.5
#bit PORTB_6	= PORTB.6
#bit PORTB_7	= PORTB.7

#bit PORTC_0	= PORTC.0
#bit PORTC_1	= PORTC.1
#bit PORTC_2	= PORTC.2
#bit PORTC_3	= PORTC.3
#bit PORTC_4	= PORTC.4
#bit PORTC_5	= PORTC.5
#bit PORTC_6	= PORTC.6
#bit PORTC_7	= PORTC.7

#bit INTEDG 	= OPTIONREG.6

