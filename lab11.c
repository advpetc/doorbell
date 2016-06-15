/********************************************
 *
 *  Name: Haoyang Chen
 *  Section: Wed 2:00-3:00
 *  Assignment: Final Project
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "lcd.h"

//volatile int frequency_num=0;
const long long MASK=250000;//16000000/64
volatile int check_selection=0;//0:not in selection; 1: in selection
volatile int count=0;
volatile int count1=0;
volatile int position=0;//current position
volatile int a=0;
volatile int b=0;
volatile unsigned char current_notes[8]={1,2,3,4,5,6,7,8};//store the current notes' index
char my_buffer[23];//buffer
volatile char flag=0;
char count_buffer=0;
#define FOSC 16000000 // Clock frequency
#define BAUD 9600 // Baud rate used
#define MYUBRR (FOSC/16/BAUD-1) // Value for UBRR0


int doorbell_display[26]={0,1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25};
int doorbell_frequency[26]={0, 131, 139, 147, 156, 165, 176, 185, 196, 208, 220, 233, 247, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523};
char doorbell_note1[26]={'A','C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B', 'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B', 'C'};
char doorbell_note2[26]={'-','3', '3', '3', '3', '3', '3', '3', '3', '3', '3', '3', '3', '4', '4', '4', '4', '4', '4', '4', '4', '4', '4', '4', '4', '5'};
char doorbell_note3[26]={' ',' ', '#', ' ', '#', ' ', ' ', '#', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', '#', ' ', '#', ' ', ' '};
char back_note[22] = {'@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q',
    'R','S','T','$'};
char frequency[26]={0, 1908, 1799, 1701, 1603, 1515, 1420, 1351, 1276, 1202, 1136, 1073, 1012, 954, 903, 850, 804, 758, 716, 676, 638, 602, 568, 536, 506, 478};

char temp[23];


void note_display(int frequency_num, int col);
void print_all_notes(int frequency_num);
void notes_play(unsigned char current_notes[]);
void note_change(int position);
void notes_back_play(char my_buffer[], int scope);
void notes_send(char back_note[], int scope);
void note_play(char position);

void init_timer1(unsigned int frequency_num)
{
    TCCR1B|=(1<<WGM12);
    TIMSK1|=(1<<OCIE1A);  //interrupt enable
    OCR1A=MASK/(2*doorbell_frequency[frequency_num]);
    TCCR1B|=(1<<CS10);  // Prescaler = 64
    TCCR1B|=(1<<CS11);
//     sei();
}

int main(void) {
    
    /* Main program goes here */
    
    //LCD port initialization
    DDRD=0b11110000;
    DDRB=0b00000011;
    
    //initialization
    init_lcd();
    init_acd();
    
    //for the output of buzzer
    DDRB|=(1<<PB3);//for the output of buzzer
    
    //set rotary input
    PORTC|=(1<<PC4);
    PORTC|=(1<<PC5);
    
    //for button pressing inputs
    //enable pull-up resistors
    PORTD|=(1<<2);//front
    PORTD|=(1<<3);//back
    
    //enable interruption to PINC
    PCICR|=(1<<PCIE1);//enable interrupts for PORTC
    PCMSK1|=((1<<PCINT12)|(1<<PCINT13));//enable interrupts for PC4 and PC5

    //serial interface
    UCSR0B|=(1<<RXCIE0);
    
    //enable golbal interrupt
    sei();
    
    //initial interface
    moveto(0,0);
    writecommand(0x01);
    char initial[32];
    snprintf(initial,32,"Haoyang Chen's ringbell");
    stringout(initial);
    stringout(initial);
    _delay_ms(1000);
    writecommand(0x01);
    
    if(eeprom_read_byte((void*)0)!=0xff)
    {
        int i=0;
        for(i=0;i<8;i++)
        {
            current_notes[i]=eeprom_read_byte((void*)i);
        }
    }
    print_all_notes(position);
    moveto(1,position);
    while (1) {      // Loop forever
        ADCSRA|=0x40;//start conversion
        while((ADCSRA&0x40))
        {
            
        }
        unsigned char x;
        x=ADCH;
        
        if((PIND&(1<<PD2))==0)//front_doorbell
        {
            writecommand(0x01);
            char temp1[8];
            snprintf(temp1,8,"front");
            stringout(temp1);
            notes_play(current_notes);
        }
        if((PIND&(1<<PD3))==0)//back_doorbell
        {
            writecommand(0x01);
            char temp2[8];
            snprintf(temp2,8,"back");
            stringout(temp2);
            
            //serial interface
            UBRR0=MYUBRR;//set baud rate
            UCSR0B |= (1 << TXEN0) | (1 << RXEN0); // Enable RX and TX
            UCSR0C = (3 << UCSZ00); // Async., no parity, 1 stop bit, 8 data bits
            DDRC|=(1<<PC3);
            PORTC&=~(1<<PC3);
            notes_send(back_note,23);
            if(flag==1)
            {
                notes_back_play(my_buffer,20);
                _delay_ms(150);
                flag=0;
            }
        }
        
        if(x>190&&x<210)//select button
        {
            if(check_selection==0)
            {
                check_selection=1;
            }
            else if(check_selection==1)
            {
                check_selection=0;
            }
        }
        
        if(check_selection==1)
        {
            note_change(position);
        }

        if(x>-10&&x<40)//right button/back
        {
            if(position==14)
            {
                position=0;
            }
            else
            {
                position=position+2;
                moveto(1,position);
                _delay_ms(1000);
            }
        }
        
        if(x>135&&x<175)//left button/front
        {
            if(position==0)
            {
                position=14;
            }
            else
            {
                position=position-2;
                moveto(1,position);
                _delay_ms(1000);
            }

        }

        eeprom_update_block(current_notes, (void*)0, 8);
    }
    return 0;   /* never reached */
}

ISR(TIMER1_COMPA_vect)
{
    PORTB^=(1<<PB3);
}

ISR(PCINT1_vect)
{
    if(count1>259)
    {
        count1=259;
    }
    if(count1<0)
    {
        count1=0;
    }
    unsigned char A=(PINC&(1<<4))>>4;  // the state of A
    unsigned char B=(PINC&(1<<5))>>5;  // the state of B
    
    if (a==0&&b==0){ // state 00
        if (A==1){ // A = 1CW
            a=1;
            count1++;
        }
        else if(B==1){ // B = 1 CCW
            b=1;
            count1--;
        }
    }
    else if(a==1&&b==0){ // state 01
        if(B==1){ // B = 1 CW
            b=1;
            count1++;
        }
        else if(A==0){ // A = 0 CCW
            a=0;
            count1--;
        }
    }
    else if(a==1&&b==1){
        if(A==0){ // A = 0 CW
            a=0;
            count1++;
        }
        else if(B==0){ // B = 0 CCW
            b=0;
            count1--;
        }	
    }
    else if(a==0&&b==1){
        if(B==0){ // B = 0 CW
            b=0;
            count1++;
        }
        else if(A==1){ // A = 1 CCW
            a=1;
            count1--;
        }	
    }
    count=count1/10;
}

ISR(USART_RX_vect)
{
    char received = UDR0;

	if(received=='@')
    {
    	count_buffer=0;
        my_buffer[count_buffer]=received;
        count_buffer=1;
        flag=0;
    }
    else if (received=='$'&& count_buffer>=0)
    {
    	my_buffer[count_buffer]=received;
        count_buffer++;
        flag=1;
    }
    else if(received>='A'&&received<='Z')
    {
    	my_buffer[count_buffer]=received;
        count_buffer++;
    }
    else
    {
    	count_buffer=0;
    }
}

void note_display(int frequency_num,int col)
{
    moveto(0,col*2);
    char display1[4];
    snprintf(display1,4,"%c%c",doorbell_note1[current_notes[frequency_num]],doorbell_note3[current_notes[frequency_num]]);
    stringout(display1);
    
    moveto(1,col*2);
    char display2[2];
    snprintf(display2,2,"%c%c",doorbell_note2[current_notes[frequency_num]],' ');
    stringout(display2);
    
}

void print_all_notes(int frequency_num)
{
    int count1=0;
    while(count1<8)
    {
        note_display(frequency_num,count1);
        count1++;
        frequency_num++;
    }
}

void notes_play(unsigned char current_notes[])
{
    int count_notes=0;//counting how many times to play the notes
    while(count_notes<8)
    {
//         if(current_notes[0]=='@';
// 	{
        init_timer1(current_notes[count_notes]);
        count_notes++;
        _delay_ms(150);
    }
    TCCR1B&=~(0b11<<CS10);
}

void note_change(int position)
{
    int position1=0;
    position1=position/2;
    current_notes[position1]=count;
    note_display(position1,position1);
    eeprom_update_byte((void*)position1,current_notes[position1]);
}

void notes_back_play(char my_buffer[], int scope)
{
    int count1_notes=1;
    while(count1_notes<=scope)
    {
        temp[count1_notes]=my_buffer[count1_notes]-'A';
		note_play(count1_notes);
		count1_notes++;						
    }
}

void notes_send(char *temp, int scope)
{
    int count2_note=0;
    while(count2_note<scope)
    {
        while ((UCSR0A&(1<<UDRE0))==0){}
        UDR0=temp[count2_note];
        count2_note++;
    }
}

void note_play(char position)
{
    init_timer1(temp[position]);
    _delay_ms(150);
    TCCR1B&=~(0b11<<CS10);
}