/*	Author: Brandon Tran
 *  Partner(s) Name: 
 *	Lab Section: 22
 *	Assignment: Lab #9  Exercise #1
 *	Exercise Description: Using the ATmega1284’s PWM functionality, design a system that uses three buttons to select one of three tones to be generated on the speaker. When a button is pressed, the tone mapped to it is generated on the speaker. Criteria:
Use the tones C4, D4, and E4 from the table in the introduction section.
When a button is pressed and held, the tone mapped to it is generated on the speaker.
When more than one button is pressed simultaneously, the speaker remains silent. 
When no buttons are pressed, the speaker remains silent.

 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link: https://youtu.be/BO9d7LDcTPw
 */
#include <avr/io.h>
#include "timer.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

// 0.954 is lowest frequency possible with this function,
// based on settings in PWM_on()
// Passing in 0 as the frequency will stop the speaker from generating sound
void set_PWM(double frequency) {
	static double current_frequency	; // Keeps track of the currently set frequency
	// Will only update the registers when the frequency changes, otherwise allows
	// music to play uninterrupted
	if(frequency != current_frequency) {
		if(!frequency) { TCCR3B &= 0x08; } // stops timer/counter
		else { TCCR3B |= 0x03; } // resumes/continues timer/counter
		
		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if(frequency < 0.954) { OCR3A = 0xFFFF; }
		
		// prevents OCR3A from underflowing, using prescaler 64
		// 31250 is largest frequency that will not result in underflow
		else if(frequency > 31250) { OCR3A = 0x0000; }
		
		// set OCR3A based on desired frequency
		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }
		
		TCNT3 = 0; // resets counter
		current_frequency = frequency; // Updates the current frequency
	}
	
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
		// COM3A0: Toggle PB3 on compare match between counter and OCR3A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
		// WGM32: When counter (TCNT3) matches OCR3A, reset counter
		// CS31 & CS30: Set a prescaler of 64
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

unsigned char cBtn, dBtn, eBtn, tmpA;

void tick() {
    tmpA = ~PINA & 0x07;
    
    cBtn = tmpA & 0x04;
    dBtn = tmpA & 0x02;
    eBtn = tmpA & 0x01;    
    
    if(cBtn && !dBtn && !eBtn) { set_PWM(261.63); }
    else if(!cBtn && dBtn && !eBtn) { set_PWM(293.66); }
    else if(!cBtn && !dBtn && eBtn) { set_PWM(329.63); }
    else { set_PWM(0); }
    
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF; // input
	DDRB = 0xFF; PORTB = 0x00; // output (pin 6 should be 1)
    /* Insert your solution below */
    PWM_on();
    
    while (1) {
    	tick();
    }
    
    return 1;
}
