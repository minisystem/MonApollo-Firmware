#ifndef CLOCK_H
#define CLOCK_H

struct clock {
	
	uint8_t divider;
	uint8_t ppqn_counter;
	uint16_t rate; //output compare value for clock timer
	uint16_t previous_rate; 
	
	
	};
	
extern struct clock midi_clock;
extern struct clock system_clock;

void setup_system_clock(void);	
void update_clock_rate(uint16_t rate);

#endif