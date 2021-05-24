/*	Author: Skyler Saltos 
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #10  Exercise #2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <stdbool.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "timer.h"
#include "keypad.h"
#include "scheduler.h"
#include "bit.h" 

#endif

bool lock = true;//global variable to relay message to locking control 

enum SM1_States { SM1_Start,SM1_waitFlag, SM1_BeginCheck, SM1_CorrectPush, SM1_PushReleased };

int LockCombination(int state ) {

	static unsigned char input; //user input from buttons(call function at start of SM) 
	input = GetKeypadKey();
	static unsigned char lockSequence[] = { '1', '2', '3' , '4', '5' }; 
	static unsigned char k = 0;

	unsigned char B7 = PINB & 0x80;

   switch(state) { // Transitions
      case SM1_Start:
         state = SM1_waitFlag;
         break;
      case SM1_waitFlag:
         if (input == '#') {
            state = SM1_BeginCheck;
         }
         else {
            state = SM1_waitFlag;
         }
         break;
      case SM1_BeginCheck:
         if (input == '\0') {
            state = SM1_BeginCheck;
         }
         else if (input == lockSequence[k]) {
            state = SM1_CorrectPush;
         }
         else if (input != lockSequence[k]) {
            state = SM1_waitFlag;
         }
         break;
      case SM1_CorrectPush:
         if (input == lockSequence[k]) {
            state = SM1_CorrectPush;
         }
         else if (input == '\0') {
            state = SM1_PushReleased;
         }
         break;
      case SM1_PushReleased:
            state = SM1_BeginCheck;
         break;
      default:
         state = SM1_Start;
      } // Transitions

   switch(state) { // State actions
      case SM1_waitFlag:
         k = 0;
         if( !B7){
         
         lock = true;
         }
	 PORTA = 0x00;
         break;
      case SM1_BeginCheck:
         if( k == 5 ) {
         
         lock = false;
	 k = 0;
         
         }//unlock Door
         
          if ( !B7 ) { //B7 pressed so lock
         
         lock = true;
         
         }//lock door 
        
	 PORTA = 0x01; 
         break;
      case SM1_CorrectPush:
//	 PORTA = 0x00;
         break;
      case SM1_PushReleased:
         ++k;
         break;
      default: // ADD default behaviour below
         break;
   } // State actions

   return state;

}//end lockCombination SM

enum SM2_States { SM2_Start,SM2_Lock, SM2_Unlock };

int LockControl(int state) {
   
   switch(state) { // Transitions
      case SM2_Start:
         state = SM2_Lock;
         break;
      case SM2_Lock:
         if (lock) {
            state = SM2_Lock;
         }
         else if (!lock) {
            state = SM2_Unlock;
         }
         break;
      case SM2_Unlock:
         if (!lock) {
            state = SM2_Unlock;
         }
         else if (lock) {
            state = SM2_Lock;
         }
         break;
      default:
         state = SM2_Start;
      } // Transitions

   switch(state) { // State actions
      case SM2_Lock:
         PORTB = PORTB & 0xFE;
         break;
      case SM2_Unlock:
         PORTB = PORTB | 0x01;
         break;
      default: // ADD default behaviour below
         break;
   } // State actions

   return state;

}//end LockControl SM









int main(void) {
    /* Insert DDR and PORT initializations */

	DDRB = 0x7F; PORTB = 0x80; // set PB[6:0] as output and PB7 as input
	DDRC = 0xF0; PORTC = 0x0F; //set PC7 - PC4 = 0 and set PC3 - PC0 = 1
	DDRA = 0xFF; PORTA = 0x00; //set PORTA as output to test variable values


	static task task1, task2; // define variable of type task for keypadSM
	task *tasks[] = { &task1, &task2 }; // array holding pointers to task objects created 
	const unsigned short numTasks = sizeof(tasks) / sizeof( task*); //hardcode the number of tasks in the scheudlre ( SM's created )


	//LockCombination
	task1.state =  SM1_Start;
	task1.period = 10;
	task1.elapsedTime = task1.period;
	task1.TickFct = &LockCombination;

	//LockControl
	task2.state = SM2_Start;
	task2.period = 1;
	task2.elapsedTime = task2.period;
	task2.TickFct = &LockControl;


	unsigned short i = 0; // for scheduler
	unsigned short j = 0; // for GCD 
	unsigned long GCD = tasks[0]->period; //value to hold initial GCD before check

	for ( j = 0; j < numTasks; j++ ) {
		GCD = findGCD(GCD, tasks[i]->period);
	}//end for to find GCD of all task periods

	TimerSet(GCD);// TODO: TEST TIMER
	TimerOn();

    while (1) {

	for(i = 0; i < numTasks; i++ ){ //scheduler code 

		if( tasks[i]->elapsedTime >= tasks[i]->period) { //task ready for tick
			tasks[i]->state = tasks[i]->TickFct(tasks[i]->state); //set next state
			tasks[i]->elapsedTime = 0; // reset elapsed time for next tick
		}//end if 

		tasks[i]->elapsedTime += GCD;

	}//end for 

	while(!TimerFlag){};
	TimerFlag = 0;

	}//end outer while loop


	    
    return 0;
}//end main
