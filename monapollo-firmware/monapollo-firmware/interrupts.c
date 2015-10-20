#include <avr/io.h>
#include <avr/interrupt.h>

#include "hardware.h"
#include "tune.h"
#include "display.h"
#include "clock.h"

ISR (TIMER0_COMP_vect) { //timer 0 output compare interrupt for tuning
	
	OCR0A = period-1; //OCR0A counts n-1 periods - see comment in tune.c about setting OCR0A
	//PORTB ^= (1<<ARP_SYNC_LED); //toggle arp ARP_SYNC LED
	if (period_counter == 0) {
		count_finished = FALSE;
		no_overflow = TRUE;
		period_counter = 1; //set period counter to 1
		//set up 16 bit timer/counter1		
		TCCR1B |= timer1_clock; //clock /64 to run at 312.5 KHz or /8 to run at 2.5 MHz, dependent on note frequency being measured
		TIMSK1 |= (1<<TOIE1); //enable timer1 overflow interrupt
		
	} else {
		
		osc_count = TCNT1;		
		TCCR1B = 0; //turn off 16 bit timer/counter1
		count_finished = TRUE;
		period_counter = 0;
		TCNT1 = 0; //reset timer/counter 1
	}
	
	
	
	
}



ISR (TIMER1_OVF_vect) {
	
	//during frequency counting, if timer1 overflow occurs set overflow flag
	no_overflow = FALSE;
	//PORTB ^= (1<<ARP_SYNC_LED); //toggle arp VCO_SYNC_LATCH_BIT LED
	
}

ISR (TIMER1_COMPA_vect) { //output compare match for master clock
	//PORTB ^= (1<<ARP_SYNC_LED);
	if (system_clock.ppqn_counter == system_clock.divider >> 1) { //50% gate width
		
		//PORTF &= ~(1<<GATE); //50% gate width
		PORTB &= ~ (1<<ARP_SYNC_LED);
	}		
	if (++system_clock.ppqn_counter == system_clock.divider) {
		//PORTB ^= (1<<ARP_SYNC_LED);
		system_clock.ppqn_counter = 0;
		PORTB |= (1<<ARP_SYNC_LED);
		//PORTF |= (1<<GATE);
	}
	
}