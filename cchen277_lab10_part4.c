/* Author: Chi Chuan Howard Chen
 * Partner(s) Name (if applicable):  
 * Lab Section: 22
 * Assignment: Lab #10  Exercise #3
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 *
 *  Demo Link: Youtube URL> 
 */

/* header files added
    asf
    bit
    io
    io
    keypad
    lcd_8bit_task
    main
    queue
    receive.hex
    scheduler
    send.hex
    seven_seg
    stack
    timer
    usart
*/

#include <avr/io.h>
// #include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

#include "bit.h"
#include "keypad.h"
#include "scheduler.h"
#include "timer.h"

unsigned char led0_output = 0x00;
unsigned char led1_output = 0x00;
unsigned char keypad_output = 0x00;
unsigned char pause = 0;

enum pauseButtonSM_States {pauseButton_wait, pauseButton_press, pauseButton_release};

int pauseButtonSMTick(int state) {
    unsigned char press = ~PINA & 0x01;

    switch(state) {
        case pauseButton_wait:
            state = press == 0x01 ? pauseButton_press : pauseButton_wait; break;
        case pauseButton_press:
            state = pauseButton_release; break;
        case pauseButton_release:
            state = press == 0x00 ? pauseButton_wait : pauseButton_press; break;
        default: state = pauseButton_wait; break;
    }
    switch(state) {
        case pauseButton_wait: break;
        case pauseButton_press:
            pause = (pause == 0) ? 1 : 0; //toggle pause
            break;
        case pauseButton_release: break;
    }
    return state;
}

enum toggleLED0_State {toggleLED0_wait, toggleLED0_blink};

int toggleLED0SMTick(int state) {
    switch(state) {
        case toggleLED0_wait: state = !pause ? toggleLED0_blink: toggleLED0_wait; break;
        case toggleLED0_blink: state = pause ? toggleLED0_wait: toggleLED0_blink; break;
        default: state = toggleLED0_wait; break;
    }
    switch(state) {
        case toggleLED0_wait: break;
        case toggleLED0_blink:
            led0_output = (led0_output == 0x00) ? 0x01 : 0x00;
            break;
    }
    return state;
}

enum toggleLED1_State {toggleLED1_wait, toggleLED1_blink};

int toggleLED1SMTick(int state) {
    switch(state) {
        case toggleLED1_wait: state = !pause ? toggleLED1_blink: toggleLED1_wait; break;
        case toggleLED1_blink: state = pause ? toggleLED1_wait: toggleLED1_blink; break;
        default: state = toggleLED1_wait; break;
    }
    switch(state) {
        case toggleLED1_wait: break;
        case toggleLED1_blink:
            led1_output = (led1_output == 0x00) ? 0x01: 0x00;
            break;
    }
    return state;
}

enum display_States {display_display};

int displaySMTick(int state) {
    //unsigned char output;

 switch(state) {
	 case display_display: state = display_display; break;
	 default: state = display_display; break;
 }
 switch (state) {
	 case display_display:
	 if (keypad_output != 0x1F) {
		 PORTB = 0x80; // 1000 0000
	 }
	 else {
		 PORTB = 0x00;
	 }
	 break;
 }
 return state;
 }

//SM for part2
unsigned char tmpB = 0x00;
unsigned char prev = 0x00;
unsigned char passcode[] = {1,2,3,4,5};
unsigned char curr[] = {0,0,0,0,0};
unsigned char pc_length = 5;
unsigned char n = 0x00;

enum lock_states {lock_reset, lock_wait, lock_press, lock_check, lock_U} lock_state;

void lockSMTick() {
    switch (lock_state) {
        case lock_reset:
            lock_state = lock_wait;
            break;
        case lock_wait:
            lock_state = (keypad_output != 0x1F) ? lock_press : lock_wait;
            break;
        case lock_press:
            lock_state = (keypad_output == 0x1F) ? lock_check : lock_press;
            break;
        case lock_check:
            if (prev == 0x0F) {
                lock_state = lock_reset;
            }
            else {
                if ((n+1) < (pc_length)) {
                    lock_state = (curr[n] == passcode [n]) ? lock_wait : lock_reset;
                    n++;
                }
                else if ((n+1) == pc_length) {
                    lock_state = (curr[n] == passcode[n]) ? lock_U : lock_reset;
                }
            }
            break;
        case lock_U:
            lock_state = (tmpB) ? lock_reset : lock_U ;
            break; 
    }

    switch (lock_state) {
        case lock_reset:
            n = 0;
            PORTB &= 0x80;
            prev = 0x00;
            break;
        case lock_wait:
            prev = 0x00;
            break;
        case lock_press:
            prev = keypad_output;
            if ((keypad_output != 0x0F) && (keypad_output != 0x0F)) {
                curr[n] = keypad_output;
            }
            break;
        case lock_check:
            break;
        case lock_U:
            PORTB |= 0x01; // don't touch B7 (input), set B0 to 1
            break;
    }
}

enum keypad_States {keypad_keypad};

int keypadSMTick(int state) {
    unsigned char x;
    switch (state) {
        case keypad_keypad:
            x = GetKeypadKey();
            switch (x) {
                case '\0': keypad_output = 0x1F; break;
                case '1': keypad_output = 0x01; break;
                case '2': keypad_output = 0x02; break;
                case '3': keypad_output = 0x03; break;
                case '4': keypad_output = 0x04; break;
                case '5': keypad_output = 0x05; break;
                case '6': keypad_output = 0x06; break;
                case '7': keypad_output = 0x07; break;
                case '8': keypad_output = 0x08; break;
                case '9': keypad_output = 0x09; break;
                case 'A': keypad_output = 0x0A; break;
                case 'B': keypad_output = 0x0B; break;
                case 'C': keypad_output = 0x0C; break;
                case 'D': keypad_output = 0x0D; break;
                case '*': keypad_output = 0x0E; break;
                case '0': keypad_output = 0x00; break;
                case '#': keypad_output = 0x0F; break;
                default: keypad_output = 0x1B; break;
            }
            state = keypad_keypad; 
            break;

        default: state = keypad_keypad; break;
    }

    switch (state) {
        default: break;
    }
    
    return state;
}

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

enum States {standby, play, check} state;

unsigned char tmpA = 0x00;
unsigned char j = 0x00;
double tune [] = {329.63, 0, 329.63, 0, 0, 0, 329.63, 0, 0, 0,
                261.63, 0, 329.63, 0, 0, 0, 392.00, 392.00, 
				392.00, 392.00, 0, 0, 0, 0, 196, 196, 196, 196, 0};

void doorbellSMTick() {
  switch(state) {
    // case init:
    //     state = standby;
    //     break;
    case standby:
        state = (tmpA) ? play : standby;
        break;
    case play:
        state = (j < 29) ? play : check;
          break;
    case check:
        state = (tmpA) ? check : standby;
        break;
    default:
        //state = init;
        state = standby;
        break;
  }

  switch(state) {
    // case init:
    //     break;
    case standby:
        j = 0x00;
        break;
    case play:
        set_PWM(tune[j]);
        j++;
        break;
    case check:
        set_PWM(0);
        break;
    default:
        break;
  }
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0x7F; PORTB = 0x80; // B7 is an input
    DDRC = 0xF0; PORTC = 0x0F;
    /* Insert your solution below */
    
    static task task1, task2, task3, task4, task5, task6;
    task *tasks[] = {&task1, &task2, &task3, &task4, &task5, &task6};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    const char start = -1;

    //Task1
    task1.state = start;
    task1.period = 50;
    task1.elapsedTime = task1.period;
    task1.TickFct = &pauseButtonSMTick;
    //Task2 
    task2.state = start;
    task2.period = 500;
    task2.elapsedTime = task2.period;
    task2.TickFct = &toggleLED0SMTick;
    //Task3
    task3.state = start;
    task3.period = 1000;
    task3.elapsedTime = task3.period;
    task3.TickFct = &toggleLED1SMTick;
    //Task4
    task4.state = start;
    task4.period = 10;
    task4.elapsedTime = task4.period;
    task4.TickFct = &lockSMTick;
    //Task5
    task5.state = start;
    task5.period = 100;
    task5.elapsedTime = task5.period;
    task5.TickFct = &keypadSMTick;
    //Task6
    task6.state = standby;
    j = 0x00;
    task6.period = 50;
    task6.elapsedTime = task6.period;
    task6.TickFct = &doorbellSMTick;

    unsigned long GCD = tasks[0]->period;
    unsigned short i;
    for (i = 1; i < numTasks; i++) {
        GCD = findGCD(GCD, tasks[i]->period);
    }

    TimerSet(GCD);
    TimerOn();
    PWM_on();

    // unsigned short i;
    while (1) {
        for (i = 0; i < numTasks; i++) {
            tmpA = ~PINA;
            tmpB = ~PINB & 0x80; // isolate B7
            if (tasks[i]->elapsedTime == tasks[i]->period) {
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += GCD;
        }

        while(!TimerFlag);
        TimerFlag = 0;
    }
    
    return 0;       
}