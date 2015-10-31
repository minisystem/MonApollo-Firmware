//clock for Arpegiator and for LFO reset syncing
//source is either midi clock or arp clock

#include <avr/io.h>

#include "clock.h"
#include "hardware.h"

struct clock midi_clock;
struct clock system_clock;
struct clock lfo_clock;

void setup_system_clock(void) {
	
	
	TCCR1B = (1<<CS10) | (1<<CS12) | (1<<WGM12);//TIMER1_DIVIDE_1024;
	TIMSK1 = (1<<OCIE1A);
	
	
	
}

void update_clock_rate(uint16_t rate) {
	
	OCR1A = rate;
	if (TCNT1 > rate) TCNT1 = rate - 1; //this prevents wrapping. setting TCNT1 = rate would cause immediate interrupt. Is that OK?
	
	
}