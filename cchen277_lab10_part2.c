/* Author: Chi Chuan Howard Chen
 * Partner(s) Name (if applicable):  
 * Lab Section: 22
 * Assignment: Lab #10  Exercise #2
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding tmpBlate or example
 * code, is my own original work.
 *
 *  Demo Link: Youtube URL> https://youtu.be/B3fp4yHdkvM
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
enum lock_states {lock_L, lock_hashP, lock_hashR, lock_oneP, lock_oneR, 
                    lock_twoP, lock_twoR, lock_threeP, lock_threeR,
                    lock_fourP, lock_fourR, lock_U} lock_state;

void lockSMTick() {
    switch (lock_state) {
        case lock_L: // curr = locked ... next = hashP ('#')
            lock_state = (keypad_output == 0x0F) ? lock_hashP : lock_L;
            break;
        case lock_hashP: // curr = hashP ('#') ... next = hashR ('')
            if (keypad_output == 0x1F) { lock_state = lock_hashR; }
            else if (keypad_output == 0x0F) { lock_state = lock_hashP; }
            else { lock_state = lock_L; }
            break;
        case lock_hashR: // curr = hashR ('') ... next = oneP ('1')
            if (keypad_output == 0x01) { lock_state = lock_oneP; }
            else if (keypad_output == 0x1F) { lock_state = lock_hashR; }
            else { lock_state = lock_L; }
            break;
        case lock_oneP: // curr = oneP ('1') ... next = oneR ('')
            if (keypad_output == 0x1F) { lock_state = lock_oneR; }
            else if (keypad_output == 0x01) { lock_state = lock_oneP; }
            else { lock_state = lock_L; }
            break;
        case lock_oneR: // curr = oneR ('') ... next = twoP ('2')
            if (keypad_output == 0x02) { lock_state = lock_twoP; }
            else if (keypad_output == 0x1F) { lock_state = lock_oneR; }
            else { lock_state = lock_L; }
            break;
        case lock_twoP: // curr = twoP ('2') ... next = twoR ('')
            if (keypad_output == 0x1F) { lock_state = lock_twoR; }
            else if (keypad_output == 0x02) { lock_state = lock_twoP; }
            else { lock_state = lock_L; }
            break;
        case lock_twoR: // curr = twoR ('') ... next = threeP ('3')
            if (keypad_output == 0x03) { lock_state = lock_threeP; }
            else if (keypad_output == 0x1F) { lock_state = lock_twoR; }
            else { lock_state = lock_L; }
            break;
        case lock_threeP: // curr = threeP ('3') ... next = threeR ('')
            if (keypad_output == 0x1F) { lock_state = lock_threeR; }
            else if (keypad_output == 0x03) { lock_state = lock_threeP; }
            else { lock_state = lock_L; }
            break;
        case lock_threeR: // curr = threeR ('') ... next = fourP ('4')
            if (keypad_output == 0x04) { lock_state = lock_fourP; }
            else if (keypad_output == 0x1F) { lock_state = lock_threeR; }
            else { lock_state = lock_L; }
            break;
        case lock_fourP: // curr = fourP ('4') ... next = fourR ('')
            if (keypad_output == 0x1F) { lock_state = lock_fourR; }
            else if (keypad_output == 0x04) { lock_state = lock_fourP; }
            else { lock_state = lock_L; }
            break;
        case lock_fourR: // curr = fourR ('') ... next = lock_U ('5')
            if (keypad_output == 0x05) { lock_state = lock_U; }
            else if (keypad_output == 0x1F) { lock_state = lock_fourR; }
            else { lock_state = lock_L; }
            break;                
        case lock_U: // curr = U (B7 = 0) ... next = lock (B7 = 1)
            lock_state = (tmpB) ? lock_L : lock_U ;
            break;

        default:
            break;
    }

    switch (lock_state) {
        case lock_L:
            PORTB &= 0x80; // don't touch B7 (input), set everything else to 0
            break;
        case lock_hashP:
            break;
        case lock_hashR:
            break;
        case lock_oneP:
            break;
        case lock_oneR:
            break;
        case lock_twoP:
            break;
        case lock_twoR:
            break;
        case lock_threeP:
            break;
        case lock_threeR:
            break;
        case lock_fourP:
            break;
        case lock_fourR:
            break;
        case lock_U:
            PORTB |= 0x01; // don't touch B7 (input), set B0 to 1
            break;

        default:
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


int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0x7F; PORTB = 0x80; // B7 is an input
    DDRC = 0xF0; PORTC = 0x0F;
    /* Insert your solution below */
    
    static task task1, task2, task3, task4, task5;
    task *tasks[] = {&task1, &task2, &task3, &task4, &task5};
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

    unsigned long GCD = tasks[0]->period;
    unsigned short i;
    for (i = 1; i < numTasks; i++) {
        GCD = findGCD(GCD, tasks[i]->period);
    }

    TimerSet(GCD);
    TimerOn();

    // unsigned short i;
    while (1) {
        for (i = 0; i < numTasks; i++) {
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