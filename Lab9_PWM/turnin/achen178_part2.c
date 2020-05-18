/*	Author: Aaron Chen 
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab 9  Exercise 2
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

typedef struct task {
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFct)(int);
} task;

static task task1, task2, task3;
task *tasks[] = { &task1, &task2, &task3 };
const unsigned short tasksNum = sizeof(tasks) / sizeof(*tasks);
const unsigned short tasksPeriod = 100;

void TimerISR() {
	unsigned char i;
	for (i = 0; i < tasksNum; ++i) {
		if (tasks[i]->elapsedTime >= tasks[i]->period) {
			tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
			tasks[i]->elapsedTime = 0;
		}
		tasks[i]->elapsedTime += tasks[i]->period;
	}
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

#define C4 261.63
#define D4 293.66
#define E4 329.63
#define F4 349.23
#define G4 392.00
#define A4 440.00
#define B4 493.88
#define C5 523.25
unsigned char currNote = 0;
const static double notes[] = { C4, D4, E4, F4, G4, A4, B4, C5 };
unsigned char playSound = 1;

enum PS_States { PS_START, PS_CTRL };
int TickFct_PlaySound(int state) {
	switch (state) { //transitions
		case PS_START:
			state = PS_CTRL;
			break;
		case PS_CTRL:
			state = PS_CTRL;
			break;
		default:
			state = PS_START;
			break;
	}
	switch (state) { //state actions
		case PS_START:
			set_PWM(0);
			break;
		case PS_CTRL:
			if (playSound) {
				set_PWM(notes[currNote]);
			} else {
				set_PWM(0);
			}
			break;
		default:
			set_PWM(0);
			break;
	}
	return state;
}

enum CN_States { CN_START, CN_WAIT, CN_UP, CN_DOWN };
int TickFct_ChangeNote(int state) {
	switch (state) { //transitions
		case CN_START:
			state = CN_WAIT;
			break;
		case CN_WAIT:
			if ((~PINA & 0x05) == 0x01) {
				state = CN_UP;
				if (currNote < 7) {
					currNote++;
				}
			} else if ((~PINA & 0x05) == 0x04) {
				state = CN_DOWN;
				if (currNote > 0) {
					currNote--;
				}
			} else {
				state = CN_WAIT;
			}
			break;
		case CN_UP:
			if (~PINA & 0x01) {
				state = CN_UP;
			} else {
				state = CN_WAIT;
			}
			break;
		case CN_DOWN:
			if (~PINA & 0x04) {
				state = CN_DOWN;
			} else {
				state = CN_WAIT;
			}
			break;
		default:
			state = CN_START;
			break;
	}
	switch (state) { //state actions
		case CN_START:
			currNote = 0;
			break;
		case CN_WAIT:
			break;
		case CN_UP:
			break;
		case CN_DOWN:
			break;
		default:
			break;
	}
	return state;
}

enum PP_States { PP_START, PP_WAIT, PP_PRESS };
int TickFct_PlayPause(int state) {
	switch (state) {
		case PP_START:
			playSound = 1;
			state = PP_WAIT;
			break;
		case PP_WAIT:
			if (~PINA & 0x02) {
				state = PP_PRESS;
				playSound = !playSound;
			} else {
				state = PP_WAIT;
			}
			break;
		case PP_PRESS:
			if (~PINA & 0x02) {
				state = PP_PRESS;
			} else {
				state = PP_WAIT;
			}
			break;
		default:
			state = PP_START;
			break;
	}
	switch (state) {
		case PP_START:
			break;
		case PP_WAIT:
			break;
		case PP_PRESS:
			break;
		default:
			break;
	}
	return state;
}


int main(void) {
    /* Insert DDR and PORT initializations */
	DDRB = 0xFF; PORTB = 0x00;
	DDRA = 0x00; PORTA = 0xFF;
    /* Insert your solution below */
	unsigned char i = 0;
	tasks[i]->state = PS_START;
	tasks[i]->period = 100;
	tasks[i]->elapsedTime = 0;
	tasks[i]->TickFct = &TickFct_PlaySound;
	i++;
	tasks[i]->state = CN_START;
	tasks[i]->period = 100;
	tasks[i]->elapsedTime = 0;
	tasks[i]->TickFct = &TickFct_ChangeNote;
	i++;
	tasks[i]->state = PP_START;
	tasks[i]->period = 100;
	tasks[i]->elapsedTime = 0;
	tasks[i]->TickFct = &TickFct_PlayPause;

	TimerSet(100);
	TimerOn();
	set_PWM(C4);
	PWM_on();	
	while (1) { }
	return 1;
}
