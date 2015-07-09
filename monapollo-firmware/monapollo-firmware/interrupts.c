#include <avr/io.h>
#include <avr/interrupt.h>

#include "hardware.h"
#include "tune.h"
#include "display.h"

ISR (TIMER0_COMP_vect) { //timer 0 output compare interrupt for tuning
	
	PORTB ^= (1<<ARP_SYNC_LED); //toggle arp sync LED
	//if (period_counter == 0) {
		//
		//no_overflow = TRUE;
		//period_counter = 1; //set period counter to 1
		////set up 16 bit timer/counter1		
		//TCCR1B |= (1<<CS11) | (1<<CS10); //clock /64 to run at 312.5 KHz
		//TIMSK1 |= (1<<TOIE1); //enable timer1 overflow interrupt
		////TCNT1 = 0; //clear timer1 16 bit register
	//} else {
		//
		//osc_count = TCNT1;
		//value_to_display = osc_count;
		//TCCR1B = 0; //turn off 16 bit timer/counter1
		////TCNT1 = 0; //clear timer1 16 bit register
		//count_finished = TRUE;
		//period_counter = 0;
	//}
	
	//TCNT1 = 0;
	
	osc_count = TCNT1;
	value_to_display = osc_count;
	TCNT1 = 0;
	
	//TCNT0 = 0; //reset timer
	//value_to_display = period_counter;
	
}

//ISR (TIMER0_COMP_vect) {
	//
	//PORTB ^= (1<<ARP_SYNC_LED); //toggle arp sync LED
	//TCNT0 = 0; //reset timer
	//period_counter = 1;
	//value_to_display = 4242;
	//
//}

ISR (TIMER1_OVF_vect) {
	
	//during frequency counting, if timer1 overflow occurs set overflow flag
	no_overflow = FALSE;
	//PORTB ^= (1<<ARP_SYNC_LED); //toggle arp sync LED
	
}