/*	Author: lab
 *  Partner(s) Name:
 *	Lab Section:
 *	Assignment: Lab #12  Exercise #Prelab
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

//------------------------------
//LED Matrix Demo SynchSM
//Period: 100 ms
//------------------------------
enum Demo_States{shift};
int Demo_Tick(int state){
  //Local Variables
  static unsigned char pattern = 0x80; //lED pattern -0: LED off; 1: LED on
  static unsigned char row = 0xFE; //Row(s) displaying pattern.
                                    //0: display pattern on row
                                    //1: do NOT display pattern on row
  //Transitions
  switch(state){
    case shift:
      break;
    default:
      state = shift;
      break;
  }
  //Actions
  switch(state){
    case shift:
      if(row == 0xEF && pattern == 0x01){ //Reset Demo
        pattern = 0x80;
        row = 0xFE;
      }else if(pattern == 0x01) { //Move LED to start of next row
        pattern = 0x80;
        row = (row << 1) | 0x01;
      }else { //Shift LED one spot to the right on current row
        pattern >>= 1;
      }
      break;
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

    static task task1;
    task *tasks[] = {&task1};
    const unsigned short numTasks = sizeof(tasks) /sizeof(task*);

    const char start = -1;

    //Task1 (Demo_Tick)
    task1.state = start; ///Task initial state
    task1.period = 100; //task period
    task1.elapsedTime = task1.period;
    task1.TickFct = &Demo_Tick;

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
