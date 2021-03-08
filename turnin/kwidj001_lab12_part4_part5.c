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

static unsigned char patternArr[3];
static unsigned char rowArr[3];
unsigned char upButton, downButton, leftButton, rightButton;

/*
For the following function. Use the following connections:

PORTC[3] connected to sReg0 : SRCLR
PORTC[2] connected to sReg0 : RCLK
PORTC[1] connected to sReg0 : SRCLK
PORTC[0] connected to sReg0 : SER

*/
void transmit_data(unsigned char data){
  int i;
  for(i = 0; i < 8; ++i){
    //Sets SRCLR to 1 allowing data to be set
    //Also clears SRCLK in preparation of  sending data
    PORTC = 0x08;
    //set SER = next bit of data to be sent.
    PORTC |= ((data>>i)&0x01);
    //set SRCLK = 1. Rising edge shifts next bit of data into the shift register
    PORTC |= 0x02;
  }
  //set RCLK = 1. Rising edge copies data from "shift" register to "storage" register
  PORTC |= 0x04;
  //clears all lines in preparation of a new transmission
  PORTC = 0x00;
}

enum buildRecStates{row1, row2, row3};
int buildRecTick(int state){

  unsigned char pattern = patternArr[0]; //lED pattern 1 in row on
  unsigned char row = rowArr[0]; //Row(s) displaying pattern.
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
  transmit_data(pattern |(row << 4)); //Pattern + row(lower nibble to upper nibble) to shift register
  //PORTD = row; //Row(s) displaying pattern
  return state;
}

enum shiftRecStates{wait, up, down, left, right};
int shiftRecTick(int state){
  switch(state){
    case wait:
      if(upButton){
        state = up;
      }else if(downButton){
        state = down;
      }else if(leftButton){
        state = left;
      }else if(rightButton){
        state = right;
      }
      else{
        state = wait;
      }
      break;
    case up: state = wait; break;
    case down: state = wait; break;
    case left: state = wait; break;
    case right: state = wait; break;
    default: state = wait;
  }
  switch(state){
    case wait: break;
    case up:
      if(rowArr[0] != 0xFE){ //top row
        rowArr[0] >> 1;
        rowArr[1] >> 1;
        rowArr[2] >> 1;
      }
      break;
    case down:
      if(rowArr[3] != 0xEF){ //bottom row
        rowArr[0] << 1;
        rowArr[1] << 1;
        rowArr[2] << 1;
      }
    break;
    case left:
      if(pattern[0] != 0xF0){
        patternArr[0] << 1;
        patternArr[1] << 1;
        patternArr[2] << 1;
      }
    break;
    case right:
      if(pattern[0] != 0x0F){
        patternArr[0] >> 1;
        patternArr[1] >> 1;
        patternArr[2] >> 1;
      }
    break;
    default: break;
  }
  return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRC = 0x00; PORTC = 0xFF; //Configure port C's 8 pins as inputs
    DDRD = 0x00; PORTD = 0xFF; //Configure port D's 8 pins as inputs

    //initialize buttons
    upButton = ~PINA & 0x01; //set up button to PA0
    downButton = ~PINA & 0x02; //set up button to PA1
    rightButton = ~PINA & 0x04; //set up button to PA2
    leftButton = ~PINA & 0x08; //set up button to PA3

    //initialize arrays
    //patternArr = {0x3C, 0x24, 0x3C};
    patternArr[0] = 0x3C;
    patternArr[1] = 0x24;
    patternArr[2] = 0x3C;
    //rowArr = {0xFD, 0xFB, 0xF7};
    rowArr[0] = 0xFD;
    rowArr[1] = 0xFB;
    rowArr[2] = 0xF7;

    static task task1, task2;
    task *tasks[] = {&task1, &task2};
    const unsigned short numTasks = sizeof(tasks) /sizeof(task*);

    const char start = -1;

    //Task1 (Demo_Tick)
    task1.state = start; ///Task initial state
    task1.period = 1; //task period
    task1.elapsedTime = task1.period;
    task1.TickFct = &buildRecTick;
    //Task2(shiftRecTick)
    task2.state = start;
    task2.period = 100; //task period
    task2.elapsedTime = task2.period;
    task2.TickFct = &shiftRecTick;

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
