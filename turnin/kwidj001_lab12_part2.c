/*	Author: lab
 *  Partner(s) Name:
 *	Lab Section:
 *	Assignment: Lab #12  Exercise #2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "scheduler.h"
#include "timer.h"
#endif

//shared variables---------
unsigned char rButton;
unsigned char lButton;
//end shared variables-----
enum lineShiftStates{wait, leftShift, rightShift};
int lineShiftTick(int state){
  //Local Variables
  static unsigned char pattern = 0x80; //lED pattern 1 in row on
  static unsigned char row = 0x00; //Row(s) displaying pattern.
                                    //0: display pattern on row
                                    //1: do NOT display pattern on row
  //Transitions
  switch(state){
    case wait:
      if(rButton){
        state = rightShift;
      }else if(lButton){
        state = leftShift;
      }else{
        state = wait;
      }
      break;
    case leftShift: state = wait; break;
    case rightShift: state = wait; break;
    default:
      state = wait;
      break;
  }
  //Actions
  switch(state){
    case wait: break;
    case leftShift:
      if(pattern != 0x80){ //if at left most column dont shift left
        pattern >>= 1;
      }
      break;
    case rightShift:
      if(pattern != 0x01){//if at right most column dont shift right
        pattern <<= 1;
      }
    default:
      break;
  }
  PORTC = pattern; //Pattern to display
  PORTD = row; //Row(s) displaying pattern
  return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRC = 0x00; PORTC = 0xFF; //Configure port C's 8 pins as inputs
    DDRD = 0x00; PORTD = 0xFF; //Configure port D's 8 pins as inputs
    DDRA = 0xFF; PORTA = 0x00; //Configure port A's 8 pins as outputs, initialize to 0

    rButton = ~PINA & 0x04; //set rightShift button to PA2
    lButton = ~PINA & 0x08; //set leftShift button to PA3

    static task task1;
    task *tasks[] = {&task1};
    const unsigned short numTasks = sizeof(tasks) /sizeof(task*);

    const char start = -1;

    //Task1 (Demo_Tick)
    task1.state = start; ///Task initial state
    task1.period = 100; //task period
    task1.elapsedTime = task1.period;
    task1.TickFct = &lineShiftTick;

    unsigned long GCD = tasks[0]->period;
    for(int i = 1; i < numTasks; i++){
      GCD = findGCD(GCD, tasks[i]->period);
    }

    //Set timer and turn on
    TimerSet(GCD);
    TimerOn();

    unsigned short i;
    /* Insert your solution below */
    while (1) {
      for(i = 0; i < numTasks; i++){
        if(tasks[i]->elapsedTime == tasks[i]->period){//task is ready to tick
          tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
          tasks[i]->elapsedTime = 0; //reset the elapsed time for next cik
        }
        tasks[i]->elapsedTime += GCD;
      }
      while(!TimerFlag);
      TimerFlag = 0;
    }
    return 0; //Error: Program should not exit!
}
