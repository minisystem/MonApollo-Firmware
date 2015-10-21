#ifndef ARP_H
#define ARP_H

enum arp_mode {
	
	UP,
	DOWN,
	UP_DOWN,
	DOWN_UP,
	RANDOM
	
	
	};
	
enum clock_source {

	MIDI_CLOCK,
	INTERNAL_CLOCK	
	
};		
	
struct arp {
	
	uint8_t range;
	enum clock_source clock_source;
	enum arp_mode mode;
	
	
	};
	
extern struct arp arp;		 


#endif