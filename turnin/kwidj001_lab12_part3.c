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

enum buildRecStates{row1, row2, row3};
int buildRecTick(int state){
  //Local Variables
  static unsigned char patternArr[3] = {0x3C, 0x24, 0x3C}; //patterns to display on
  static unsigned char rowArr[3] = {0xFD, 0xFB, 0xF7};

  static unsigned char pattern = patternArr[0]; //lED pattern 1 in row on
  static unsigned char row = rowArr[0]; //Row(s) displaying pattern.
                                    //0: display pattern on row
                                    //1: do NOT display pattern on row
  //Transitions
  switch(state){
    case row1: state = row2; break;
    case row2: state = row3; break;
    case row3: state = row1; break;
    default: state = row1; break;
  }
  //Actions
  switch(state){
    case row1:
      pattern = patternArr[0];
      row = rowArr[0];
      break;
    case row2:
      pattern = patternArr[1];
      row = rowArr[1];
      break;
    case row3:
      pattern = patternArr[2];
      row = rowArr[2];
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

    static task task1;
    task *tasks[] = {&task1};
    const unsigned short numTasks = sizeof(tasks) /sizeof(task*);

    const char start = -1;

    //Task1 (Demo_Tick)
    task1.state = start; ///Task initial state
    task1.period = 1; //task period
    task1.elapsedTime = task1.period;
    task1.TickFct = &buildRecTick;

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
