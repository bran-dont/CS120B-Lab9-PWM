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
 *	Demo Link: https://youtu.be/3eWNgALtoKw
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

enum States { Wait, Play, Press } state;
double notes[10] = { 261.63, 329.63, 349.23, 440.00, 349.23, 329.63, 261.63, 493.88, 293.66, 261.63 }; // C E F A F E C~ B D C
//320, 320, 320, 320, 320, 320, 950, 160, 160, 1270
double hold[10] = {40, 40, 40, 40, 40, 40, 100, 20, 20, 130 };
double gap[9] = { 7, 7, 7, 7, 7, 7, 7, 7, 7 };
unsigned char tmpA, i, play, n;

void tick() {
    tmpA = ~PINA & 0x01;
    
    switch(state) {
    	case Wait:
    		if(tmpA == 0x01) {
    			state = Play;
    			play = 1;
    		}
    	break;
    	
    	case Play:
    		if(n == 10) {
    			state = (tmpA == 0x01 ? Press : Wait);
    			i = 0;
    			n = 0;
    		} 
    	break;
    	
    	case Press:
    		state = (tmpA == 0x01 ? Press : Wait);
    	break;
    	
    	default:
    	break;
    }
    
    switch(state) {
    	case Wait:
    	break;
    	
    	case Play:
    		if(play){
    			if(i < hold[n]) {
    				set_PWM(notes[n]);
    				i++;
    			}
    			else{
    				i = 0;
    				play = 0;
    			}
    		}
    		else {
    			if(i < gap[n]) {
    				set_PWM(0);
    				i++;
    			}
    			else {
    				n++;
    				i = 0;
    				play = 1;
    			}
    		}
    	break;
    	
    	case Press:
    	break;
    	
    	default:
    	break;
    }
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF; // input
	DDRB = 0xFF; PORTB = 0x00; // output (pin 6 should be 1)
    /* Insert your solution below */
    TimerSet(10);
    TimerOn();
    PWM_on();
    
    play = 1;
    i = 0;
    n = 0;
    
    while (1) {
    	if(TimerFlag){
    		tick();
    		TimerFlag = 0;
    	}
    }
    
    return 1;
}
