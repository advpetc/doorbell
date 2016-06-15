/********************************************
*
*  Name: Haoyang Chen
*  Section: Wed 2:00-3:00
*  Assignment: Lab 6 - Write to LCD display
*
********************************************/
#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"
void writenibble(unsigned char);
#define MASKBITS 0xf0

/*
  init_lcd - Do various things to initialize the LCD display
*/
void init_lcd()
{
    _delay_ms(15);              // Delay at least 15ms

    /* ??? */ 
    writenibble(0b00000011);    // Use writenibble to send 0011
    _delay_ms(5);               // Delay at least 4msec

    /* ??? */     
	writenibble(0b00000011);   	// Use writenibble to send 0011
    _delay_us(120);             // Delay at least 100usec

	writenibble(0b00000011); 	// Use writenibble to send 0011, no delay needed
    /* ??? */        
    writenibble(0b00000010);	// Use writenibble to send 0010
    _delay_ms(2);               // Delay at least 2ms
    
    writecommand(0x28);         // Function Set: 4-bit interface, 2 lines

    writecommand(0x0f);         // Display and cursor on

}

void init_acd()
{
//initiate ACD
//    ADMUX |= (1<<REFS0);
//    ADMUX |= (0b00<<MUX0);
//    ADMUX |= (1<<ADLAR);
//    ADCSRA |= (0b111<<ADPS0);
//    ADCSRA |= (1<<ADSC);
//    ADCSRA |= (1<<ADEN);

    ADMUX=0x60;
    ADCSRA=0x87;
}

/*
  moveto - Move the cursor to the row and column given by the arguments.
*/
void moveto(unsigned char row, unsigned char col)
{
	if(row==0)	
	{
		writecommand(0x80+col);
	}
	if(row==1)
	{
		writecommand(0xc0+col);
	}
}
/*
  stringout - Print the contents of the character string "str"
  at the current cursor position.
*/
void stringout(char *str)
{
	//writedata(*str);
	writecommand(0x0f);
	int counter=0;
	
	while(str[counter]!='\0')
	{
		writedata(str[counter]);
		counter++;
	}	
}
/*
  writecommand - Output a byte to the LCD command register.
*/
void writecommand(unsigned char x)
{
	PORTB = 0b00;
	writenibble(x>>4);
	writenibble(x);
	_delay_ms(2);
}

/*
  writedata - Output a byte to the LCD data register
*/
void writedata(unsigned char x)
{
	PORTB = 0b01;
	writenibble(x>>4);
	writenibble(x);
	_delay_ms(2);
}

/*
  writenibble - Output four bits from "x" to the LCD
*/
void writenibble(unsigned char x)
{
	PORTD &=~ MASKBITS;
    PORTD |= (MASKBITS & (x<<4));
	PORTB |= (1<<PB1);//enable line
	PORTB |= (1<<PB1);
	PORTB &= ~(1<<PB1);
}