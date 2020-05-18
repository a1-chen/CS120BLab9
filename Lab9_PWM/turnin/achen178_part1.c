/*	Author: Aaron Chen
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab 9  Exercise 1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include <avr/interrupt.h>

volatile unsigned char TimerFlag = 0; //TimerISR() sets to 1. C programmer should clear to 0.

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
	TCCR1B = 0x0B;

	OCR1A = 125;

	TIMSK1 = 0x02;

	TCNT1 = 0;

	_avr_timer_cntcurr = _avr_timer_M;

	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00;
}

void TimerISR() {
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void set_PWM(double frequency) {
	static double current_frequency;
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; }
		else { TCCR3B |= 0x03; }

		if (frequency < 0.954) { OCR3A = 0xFFFF; }

		else if (frequency > 31250) { OCR3A = 0x0000; }

		else { OCR3A = (short) (8000000 / (128 * frequency)) - 1; }

		TCNT3 = 0;
		current_frequency = frequency;
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

enum States { START, WAIT, MULT_PRESS, C_PRESS, D_PRESS, E_PRESS } state;
#define C4 261.63
#define D4 293.66
#define E4 329.63
#define F4 349.23
#define G4 392.00
#define A4 440.00
#define B4 493.88
#define C5 523.25

void Tick() {
	switch (state) { //transitions
		case START:
			state = WAIT;
			break;
		case WAIT:
			if ((~(PINA) & 0x01) && !(~(PINA) & 0x02) && !(~(PINA) & 0x04)) {
				state = C_PRESS;
			} else if (!(~(PINA) & 0x01) && (~(PINA) & 0x02) && !(~(PINA) & 0x04)) {
				state = D_PRESS;
			} else if (!(~(PINA) & 0x01) && !(~(PINA) & 0x02) && (~(PINA) & 0x04)) {
				state = E_PRESS;
			} else {
				state = WAIT;
			}
			break;
		case C_PRESS:
			if ((~(PINA) & 0x01) && !(~(PINA) & 0x02) && !(~(PINA) & 0x04)) {
				state = C_PRESS;
			} else if (!(~(PINA) & 0x01) && (~(PINA) & 0x02) && !(~(PINA) & 0x04)) {
				state = D_PRESS;
			} else if (!(~(PINA) & 0x01) && !(~(PINA) & 0x02) && (~(PINA) & 0x04)) {
				state = E_PRESS;
			} else {
				state = WAIT;
			}
			break;
		case D_PRESS:
			if ((~(PINA) & 0x01) && !(~(PINA) & 0x02) && !(~(PINA) & 0x04)) {
				state = C_PRESS;
			} else if (!(~(PINA) & 0x01) && (~(PINA) & 0x02) && !(~(PINA) & 0x04)) {
				state = D_PRESS;
			} else if (!(~(PINA) & 0x01) && !(~(PINA) & 0x02) && (~(PINA) & 0x04)) {
				state = E_PRESS;
			} else {
				state = WAIT;
			}
			break;
		case E_PRESS:
			if ((~(PINA) & 0x01) && !(~(PINA) & 0x02) && !(~(PINA) & 0x04)) {
				state = C_PRESS;
			} else if (!(~(PINA) & 0x01) && (~(PINA) & 0x02) && !(~(PINA) & 0x04)) {
				state = D_PRESS;
			} else if (!(~(PINA) & 0x01) && !(~(PINA) & 0x02) && (~(PINA) & 0x04)) {
				state = E_PRESS;
			} else {
				state = WAIT;
			}
			break;
		default:
			state = START;
			break;
	}
	switch (state) { //state actions
		case START:
			set_PWM(0);
			break;
		case WAIT:
			set_PWM(0);
			break;
		case C_PRESS:
			set_PWM(C4);
			break;
		case D_PRESS:
			set_PWM(D4);
			break;
		case E_PRESS:
			set_PWM(E4);
			break;
		default:
			set_PWM(0);
			break;
	}
}


int main(void) {
    /* Insert DDR and PORT initializations */
	DDRB = 0xFF; PORTB = 0x00;
	DDRA = 0x00; PORTA = 0xFF;
    /* Insert your solution below */
	state = START;

	PWM_on();	
	while (1) {
		Tick();
	}
	return 1;
}
