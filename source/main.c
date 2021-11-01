/*	Author: Brandon Tran
 *  Partner(s) Name: 
 *	Lab Section: 22
 *	Assignment: Lab #9  Exercise #2
 *	Exercise Description: Using the ATmega1284’s PWM functionality, design a system where the notes: C4, D, E, F, G, A, B, and C5,  from the table at the top of the lab, can be generated on the speaker by scaling up or down the eight note scale. Three buttons are used to control the system. One button toggles sound on/off. The other two buttons scale up, or down, the eight note scale. Criteria:
The system should scale up/down one note per button press.
When scaling down, the system should not scale below a C.
When scaling up, the system should not scale above a C.


 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
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

double notes[] = { 261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25 };
enum States { Wait, Dec, Tog, Inc } state;
unsigned char on, tmpA, i;

void tick() {
	tmpA = ~PINA & 0x07;
	
	switch(state) {
		case Wait:
			if(tmpA == 0x04 && 0 < i) { 
				state = Dec; 
				--i;
				set_PWM(notes[i]);
			}
			else if(tmpA == 0x02) { 
				state = Tog;
				if(!on) { PWM_on(); set_PWM(notes[i]); on = 1; }
				else { PWM_off(); on = 0; }
			}
			else if(tmpA == 0x01 && i < 7) { 
				state = Inc;
				++i;
				set_PWM(notes[i]); 
			}
		break;
		
		case Dec:
			if(tmpA == 0x00) { state = Wait; }
			else if(tmpA == 0x04) {}
			else if(tmpA == 0x02) { 
				state = Tog; 
				if(!on) { PWM_on(); set_PWM(notes[i]); on = 1; }
				else { PWM_off(); on = 0; }
			}
			else if(tmpA == 0x01 && i < 7) { 
				state = Inc; 
				++i;
				set_PWM(notes[i]); 
			}
		break;
		
		case Tog:
			if(tmpA == 0x00) { state = Wait; }
			else if(tmpA == 0x04 && 0 < i) { 
				state = Dec; 
				--i;
				set_PWM(notes[i]);
			}
			else if(tmpA == 0x02) {}
			else if(tmpA == 0x01 && i < 7) { 
				state = Inc;
				++i;
				set_PWM(notes[i]); 
			}
		break;
		
		case Inc:
			if(tmpA == 0x00) { state = Wait; }
			else if(tmpA == 0x04 && 0 < i) { 
				state = Dec; 
				--i;
				set_PWM(notes[i]);
			}
			else if(tmpA == 0x02) { 
				state = Tog; 
				if(!on) { PWM_on(); set_PWM(notes[i]); on = 1; }
				else { PWM_off(); on = 0; }
			}
			else if(tmpA == 0x01 && i < 7) {}
		break;
		
		default:
		break;
	}
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF; // input
	DDRB = 0xFF; PORTB = 0x00; // output (pin 6 should be 1)
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
    /* Insert your solution below */
    TimerSet(100);
    TimerOn();
    
    state = Wait;
    on = 0;
    i = 0;
    set_PWM(261.63);
    
    while (1) {
    	if(TimerFlag) {
    		tick();
    		TimerFlag = 0;
		}
    }
    
    return 1;
}
