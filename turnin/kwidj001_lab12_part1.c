/*	Author: lab
 *  Partner(s) Name:
 *	Lab Section:
 *	Assignment: Lab #12  Exercise #1
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
unsigned char incButton;
unsigned char decButton;
//end shared variables-----
enum lineShiftStates{wait, upShift, downShift};
int lineShiftTick(int state){
  //Local Variables
  static unsigned char pattern = 0xFF; //lED pattern entire row on
  static unsigned char row = 0xFE; //Row(s) displaying pattern.
                                    //0: display pattern on row
                                    //1: do NOT display pattern on row
  //Transitions
  switch(state){
    case wait:
      if(incButton){
        state = upShift;
      }else if(decButton){
        state = downShift;
      }else{
        state = wait;
      }
      break;
    case upShift:
      state = wait;
    case downShift:
      state = wait;
    default:
      state = wait;
      break;
  }
  //Actions
  switch(state){
    case wait: break;
    case upShift:
      if(row == 0xFE){ //if at top go to bottom
        row = 0xEF;
      }else{
        row = (row >> 1) | 0x01; //shift to next row up
      }
      break;
    case downShift:
      if(row == 0xEF){ //if at bottom go to top
        row = 0xFE;
      }else{
        row = (row << 1) | 0x01; //shift next row down
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

    incButton = ~PINA & 0x01; //set increase button to PA0
    decButton = ~PINA & 0x02; //set decrease button to PA1

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
