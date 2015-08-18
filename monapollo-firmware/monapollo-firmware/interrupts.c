#include <avr/io.h>
#include <avr/interrupt.h>

#include "hardware.h"
#include "tune.h"
#include "display.h"

ISR (TIMER0_COMP_vect) { //timer 0 output compare interrupt for tuning
	compare_match_counter++;
	OCR0A = period-1; //OCR0A counts n-1 periods - see comment in tune.c about setting OCR0A
	PORTB ^= (1<<ARP_SYNC_LED); //toggle arp sync LED
	if (period_counter == 0) {
		count_finished = FALSE;
		no_overflow = TRUE;
		period_counter = 1; //set period counter to 1
		//set up 16 bit timer/counter1		
		TCCR1B |= timer1_clock; //clock /64 to run at 312.5 KHz or /8 to run at 2.5 MHz, dependent on note frequency being measured
		TIMSK1 |= (1<<TOIE1); //enable timer1 overflow interrupt
	} else {
		
		osc_count = TCNT1;
		value_to_display = compare_match_counter;//osc_count;
		TCCR1B = 0; //turn off 16 bit timer/counter1
		count_finished = TRUE;
		period_counter = 0;
		TCNT1 = 0; //reset timer/counter 1
	}
	
	
	
	
}



ISR (TIMER1_OVF_vect) {
	
	//during frequency counting, if timer1 overflow occurs set overflow flag
	no_overflow = FALSE;
	//PORTB ^= (1<<ARP_SYNC_LED); //toggle arp sync LED
	
}