/*	Author: Skyler Saltos 
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #10  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "timer.h"
//#include "keypad.h"
#include "scheduler.h"
#include "bit.h" 
#endif




unsigned char GetKeypadKey(){

	//check column 1
	PORTC = 0xEF; 
	asm("nop"); 
	
	if(GetBit(PINC,0) == 0) { return('1'); } 
	if(GetBit(PINC,1) == 0) { return('4'); }
       	if(GetBit(PINC,2) == 0) { return('7'); }
        if(GetBit(PINC,3) == 0) { return('*'); }	
	
	//check column 2
	PORTC = 0xDF;
	asm("nop");

	if(GetBit(PINC,0) == 0) { return('2'); }
	if(GetBit(PINC,1) == 0) { return('5'); }
	if(GetBit(PINC,2) == 0) { return('8'); }
	if(GetBit(PINC,3) == 0) { return('0'); }	


	//check column 3
	PORTC = 0xBF;
	asm("nop");

	if(GetBit(PINC,0) == 0) { return('3'); }
	if(GetBit(PINC,1) == 0) { return('6'); }
	if(GetBit(PINC,2) == 0) { return('9'); }
	if(GetBit(PINC,3) == 0) { return('#'); }




	//check column 3
	PORTC = 0x7F;
	asm("nop");

	if(GetBit(PINC,0) == 0) { return('A'); }
	if(GetBit(PINC,1) == 0) { return('B'); }
	if(GetBit(PINC,2) == 0) { return('C'); }
	if(GetBit(PINC,3) == 0) { return('D'); }




	return ('\0'); //default value

}//get the key that the user entered 





enum SM1_States { SM1_Start, SM1_Off, SM1_On };

int LightToggle( int state ) {

 unsigned char input;

input = GetKeypadKey();


switch(state){//transitions

	case SM1_Start:
	    state = SM1_Off;
	    break;

	case SM1_Off:
	    if( input == '1' || input == '2' || input == '3' || input == '4' || input == '5' || input == '6' || input == '7' || input == '8'|| input == '9' || input == 'A' 
			    || input == 'B' || input == 'C' || input == 'D' || input == '*' || input == '#'|| input == '0' ) 
	    { 

		    state = SM1_On;


	    }//one is on so light  

	    else {

		state = SM1_Off;
	    }//none are on so remain off

	    break;


	case SM1_On:


	    if( input != '1' && input != '2' && input != '3' && input != '4' && input != '5' && input != '6' && input != '7' && input != '8'&& input != '9' && input != 'A'
                           && input != 'B' && input != 'C' && input != 'D' && input != '*' && input != '#'&& input != '0' )
            {

                    state = SM1_Off;


            }//none one so turn off

	    else {
		    state = SM1_On;

	    }//at least one on so remain in on

	break;

}//end transitions switch




	switch(state) { //state actions
	
		case SM1_Start:
			break;

		case SM1_On:
			PORTB = 0x80;
			break;
		case SM1_Off:
			PORTB = 0x00;
			break;



	}//end actions 

return state;


}//end ligth toggle SM










int main(void) {
    /* Insert DDR and PORT initializations */

	DDRB = 0xFF; PORTB = 0x00; // set PORTB as output
	DDRC = 0xF0; PORTC = 0x0F; //set PC7 - PC4 = 0 and set PC3 - PC0 = 1


	static task task1; // define variable of type task
        task *tasks[] = { &task1 }; // array holding pointers to task objects created 
        const unsigned short numTasks = sizeof(tasks) / sizeof( task*); //hardcode the number of tasks in the scheudlre ( SM's created )


        //KeyPadTask
        task1.state =  SM1_Start;
        task1.period = 1;
        task1.elapsedTime = task1.period;
        task1.TickFct = &LightToggle;



	unsigned short i = 0; // for scheduler
        unsigned short j = 0; // for GCD 
        unsigned long GCD = tasks[0]->period; //value to hold initial GCD before check

        for ( j = 0; j < numTasks; j++ ) {
                GCD = findGCD(GCD, tasks[i]->period);
        }//end for to find GCD of all task periods

        TimerSet(GCD);// 
        TimerOn();


    /* Insert your solution below */
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

	    
    }//end while loop
    return 1;
}//end main
