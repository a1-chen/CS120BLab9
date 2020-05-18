/*	Author: Aaron Chen
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab 9  Exercise 3
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

static task task1, task2;
task *tasks[] = { &task1, &task2 };
const unsigned short tasksNum = sizeof(tasks) / sizeof(*tasks);
const unsigned short tasksPeriod = 75;
/*
void TimerISR() {
	unsigned char i;
	for (i = 0; i < tasksNum; ++i) {
		if (tasks[i]->elapsedTime >= tasks[i]->period) {
			tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
			tasks[i]->elapsedTime = 0;
		}
		tasks[i]->elapsedTime += tasks[i]->period;
	}
}*/

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

#define C4 261.63
#define D4 293.66
#define E4 329.63
#define F4 349.23
#define G4 392.00
#define A4 440.00
#define B4 493.88
#define C5 523.25
const static double notes[] = { E4, D4, C4, 0, E4, D4, C4, 0, C4, 0, C4, 0, C4, 0, C4, 0, D4, 0, D4, 0, D4, 0, D4, 0, E4, D4, C4, 0 };
const static unsigned short times[] = { 4, 4, 4, 4, 4, 4, 4, 4, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 4, 4, 4, 4 };
static unsigned char currPos = 0;
static unsigned char currTime = 0;
unsigned char playSong = 1;

enum PS_States { PS_START, PS_WAIT, PS_PLAY } state;
void TickFct_PlaySong() {
	switch (state) {
		case PS_START:
			currPos = 0;
			currTime = 0;
			state = PS_WAIT;
			break;
		case PS_WAIT:
			if (~PINA & 0x01 && !playSong) {
				playSong = 1;
				currPos = 0;
				currTime = 0;
				state = PS_PLAY;
			} else {
				state = PS_WAIT;
				if (!(~PINA & 0x01) && playSong) {
					playSong = 0;
				}
			}
			break;
		case PS_PLAY:
			if (currPos >= 28) {
				state = PS_WAIT;
			} else {
				if (currTime < times[currPos]) {
					currTime++;
				} else {
					currTime = 0;
					currPos++;
				}
				state = PS_PLAY;
			}
			break;
		default:
			state = PS_START;
			break;
	}
	switch (state) {
		case PS_START:
			break;
		case PS_WAIT:
			set_PWM(0);
			break;
		case PS_PLAY:
			set_PWM(notes[currPos]);
			break;
		default:
			set_PWM(0);
			break;
	}
}

enum BP_States { BP_START, BP_WAIT, BP_PRESS };
int TickFct_ButtonPress(int state) {
	switch (state) {
		case BP_START:
			playSong = 0;
			state = BP_WAIT;
			break;
		case BP_WAIT:
			if (~PINA & 0x01) {
				playSong = 1;
				state = BP_PRESS;
			} else {
				state = BP_WAIT;
			}
			break;
		case BP_PRESS:
			if (~PINA & 0x01) {
				state = BP_PRESS;
			} else {
				state = BP_WAIT;
				playSong = 0;
			}
			break;
		default:
			state = BP_WAIT;
			break;
	}
	switch (state) {
		case BP_START:
			break;
		case BP_WAIT:
			break;
		case BP_PRESS:
			break;
		default:
			break;
	}
}


int main(void) {
    /* Insert DDR and PORT initializations */
	DDRB = 0xFF; PORTB = 0x00;
	DDRA = 0x00; PORTA = 0xFF;
    /* Insert your solution below */
	unsigned char i = 0;
	tasks[i]->state = PS_START;
	tasks[i]->period = tasksPeriod;
	tasks[i]->elapsedTime = 0;
	tasks[i]->TickFct = &TickFct_PlaySong;
	i++;
	tasks[i]->state = BP_START;
	tasks[i]->period = tasksPeriod;
	tasks[i]->elapsedTime = 0;
	tasks[i]->TickFct = &TickFct_ButtonPress;
	
	playSong = 1;
	TimerSet(75);
	TimerOn();
	set_PWM(0);
	PWM_on();	
	while (1) {
		TickFct_PlaySong();
		while (!TimerFlag);
		TimerFlag = 0;
	}
	return 1;
}
