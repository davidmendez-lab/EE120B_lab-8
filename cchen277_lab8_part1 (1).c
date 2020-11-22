/* Author: Chi Chuan Howard Chen
 * Partner(s) Name (if applicable):  
 * Lab Section: 22
 * Assignment: Lab #8  Exercise #1
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 *
 *  Demo Link: Youtube URL> 
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

void set_PWM(double frequency) {
    static double current_frequency;

    if (frequency != current_frequency) {
        if (!frequency) {
            TCCR3B &= 0x08;
        }
        else {
            TCCR3B |= 0x03;
        }
        if (frequency < 0.954){
            OCR3A = 0xFFFF;
        }
        else if (frequency > 31250) {
            OCR3A = 0x0000;
        }
        else {
            OCR3A = (short)(8000000 / (128 * frequency)) - 1;
        }

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

int main(void) {
    // /* Insert DDR and PORT initializations */
		// 0x00 == input, 0xFF == output
		DDRA = 0x00; PORTA = 0xFF;
		DDRB = 0xFF; PORTB = 0x00;
    // /* Insert your solution below */    	
        unsigned char tmpA = 0x00;
        PWM_on();
    
    	while (1) {
            tmpA = ~PINA & 0x07;

            if (tmpA == 0x01) {
                set_PWM(261.63); //C
                PORTB = 0x06;
            }
            else if (tmpA == 0x02) {
                set_PWM(293.66); //D
                PORTB = 0x06;
            }
            else if (tmpA == 0x04) {
                set_PWM(329.63); //E
                PORTB = 0x06;
            }
            else {
                PORTB = 0x00;
            }

    	}
        PWM_off();
    return 1;
}