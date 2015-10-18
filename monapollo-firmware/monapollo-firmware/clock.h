#ifndef CLOCK_H
#define CLOCK_H

struct system_clock {
	
	uint8_t divider; //will need to separate this into LFO sync divider and ARP sync divider
	uint8_t arp_ppqn_counter;
	uint8_t midi_ppqn_counter;
	
	
	
	};
	
extern struct system_clock clock;	

#endif