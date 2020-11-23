/* Author: Chi Chuan Howard Chen
 * Partner(s) Name (if applicable):  
 * Lab Section: 22
 * Assignment: Lab #9  Exercise #3
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 *
 *  Demo Link: Youtube URL> 
 */

 #include <avr/io.h>
 #include <avr/interrupt.h>
 #ifdef _SIMULATE_
 #include "simAVRHeader.h"
 #endif

/*timer*/
volatile unsigned char TimerFlag = 0;

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
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

// The "enable global interrupts" line is SREG |= 0x80, not SREG |- 0x80
void TimerOff() {
    TCCR1B = 0x00;
}

void TimerISR () {
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

/*fsm*/
enum tl_States {tl_zero, tl_one, tl_two} tl_state;
enum bl_States {bl_three, bl_zero} bl_state;
enum sp_States {sp_on, sp_off} sp_state;
enum cl_States {cl_combine} cl_state;

unsigned char tl_led = 0x00; //ThreeLED
unsigned char bl_led = 0x00; //BlinkingLED
unsigned char sound = 0x00; //Speaker
unsigned char threeLEDs = 0x00; //CombinedLED


unsigned char tmpA = 0x00;

void ThreeLEDsSM() {
  switch(tl_state) {
    case tl_zero:
        tl_state = tl_one;
        break;
    case tl_one:
        tl_state = tl_two;
        break;
	case tl_two:
		tl_state = tl_zero;

    default:
        tl_state = tl_zero;
        break;
  }

  switch(tl_state) {
    case tl_zero:
        tl_led = 0x01;
        break;
    case tl_one:
        tl_led = 0x02;
        break;
    case tl_two:
        tl_led = 0x04;
        break;

    default:
        tl_led |= 0x01;
        break;
  }
}

void BlinkingLEDSM() {
  switch(bl_state) {
    case bl_three:
        bl_state = bl_zero;
        break;
    case bl_zero:
        bl_state = bl_three;
        break;

    default:
        break;
  }

  switch(bl_state) {
    case bl_three:
        bl_led = 0x08;
        break;
    case bl_zero:
        bl_led = 0x01;
        break;

    default:
        bl_led = 0x08;
        break;
  }
}

void SpeakerSM() {
    switch(sp_state) {
    case sp_on:
        sp_state = sp_off;
        break;
    case sp_off:
        sp_state = sp_on;
        break;

    default:
        sp_state = sp_on;
        break;
    }

    switch (sp_state){
    case sp_on:
        sound = (tmpA == 0x04) ? 0x10 : 0x00;
        break;
    case sp_off:
		sound = 0x00;
        break;

    default:
        sound = 0x10;
        break;
    }
}

void CombineLEDsSM() {
  switch(cl_state) {
    case cl_combine:
        cl_state = cl_combine;
        break;

    default:
        cl_state = cl_combine;
        break;
  }

  switch(cl_state) {
    case cl_combine:
		threeLEDs = tl_led | bl_led;
		PORTB = threeLEDs | sound;
        break;

    default:
        threeLEDs = tl_led | bl_led;
        PORTB = threeLEDs | sound;
        break;
  }
}

int main(void) {
    // /* Insert DDR and PORT initializations */
        // 0x00 == input, 0xFF == output
        DDRA = 0x00; PORTA = 0xFF;
        DDRB = 0xFF; PORTB = 0x00;
    // /* Insert your solution below */
		unsigned long tl_elapsedTime = 0; 
		unsigned long bl_elapsedTime = 0; 
		unsigned long cl_elapsedTime = 0; 
        unsigned long sp_elapsedTime = 0;
		const unsigned long timerPeriod = 1;

        tl_state = tl_zero;
        bl_state = bl_three;
        cl_state = cl_combine;
	 
        TimerSet(timerPeriod);
        TimerOn();

        while (1) {
			tmpA = ~PINA & 0x04;
			if (tl_elapsedTime >= 300) {
				ThreeLEDsSM();
				tl_elapsedTime = 0;
			}
			if (bl_elapsedTime >= 1000) {
				BlinkingLEDSM();
				bl_elapsedTime = 0;	
			}
			if (sp_elapsedTime >= 2) {
				SpeakerSM();
				sp_elapsedTime = 0;
			}
			CombineLEDsSM();

            while (!TimerFlag);
            TimerFlag = 0;
			
			tl_elapsedTime += timerPeriod;
			bl_elapsedTime += timerPeriod;
            sp_elapsedTime += timerPeriod;
        }
    return 1;
}
