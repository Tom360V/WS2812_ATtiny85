/*
 * Timer.c
 *
 * Created: 2017-01-19 14:58:43
 *  Author: tkoene
 */ 

#include <avr/io.h>
#include "Timer.h"


void timer0_Config()
{
	timer_Stop();
	//Activate SYNC mode
	GTCCR = (1<<TSM);

	//Clear timer registers
	TCCR0A = 0x00;
	TCCR0B = 0x00;
	
	//Timer, toggle OC0B on compare match
	TCCR0A |= (1<<COM0A1) | (0<<COM0A0);
	TCCR0A |= (1<<COM0B1) | (0<<COM0B0);

	//Timer, set CTC mode
	TCCR0A |= (1<<WGM01) | (1<<WGM00);
	TCCR0B |= (0<<WGM02);

	//Timer, set CTC mode
//	TCCR0B |= (0<<CS02) | (0<<CS01) | (0<<CS00);

	//Timer, enable interrupt when compare match
	TIMSK |= (1<<OCIE0B);
	
	// set max TOP
	OCR0B = 0xFF;

	//Exit SYNC mode
	GTCCR = (0<<TSM);
}

void timer1_Config()
{
	TCCR0B &= 0xF8;	//STOP TIMER 0!!!

	//Timer, enable interrupt when compare match
	TIMSK |= (1<<OCIE1A);	//Compare Match
//	TIMSK |= (1<<TOIE1);	//Over Flow

	OCR1C = 5;

	TCCR1  = (1<<CTC1) | (1<<PWM1A);
	TCCR1 |= (1<<COM1A1) | (1<<COM1A0);	//OC1x is cleared on compare match, set when TCNT1 == 0x00;
	TCCR1 |= (1<<PWM1A);
	TCCR1 |= (1<<CTC1);
	OCR1C = 10; // Frequency
	OCR1A = (OCR1C /2);
	
	timer_Start();

	PLLCSR = (0<<LSM) | (1<<PCKE) | (1<<PLLE);

}

void timer_NewPeriod(uint8_t period)
{
	//period;
	OCR0B = period; // set new TOP
	TCNT0 = 0;
}

void timer_Start()
{
	TCCR1 |= (1<<CS10);
}

void timer_Stop()
{
	TCCR0B &= 0xF8;	//clear lower 3 bits (CSnn)
}
