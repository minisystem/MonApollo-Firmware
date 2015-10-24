#ifndef ARP_H
#define ARP_H

#include "assigner.h"

enum arp_mode {
	
	UP,
	DOWN,
	UP_DOWN,
	DOWN_UP,
	RANDOM
	
	};
	
enum clock_source {

	MIDI_CLOCK,
	INTERNAL_CLOCK,
	OFF	
	
};		
	
struct arp {
	
	uint8_t range; //range from 0 to 3 octaves
	uint8_t step_number; //number of steps in arp sequence = range * number of notes held
	uint8_t step_position; //position through range
	
	uint8_t current_note; //current note in arp sequence
	
	enum clock_source clock_source;
	enum arp_mode mode;
	
	struct midi_note sequence[32]; //max is gate_buffer + gate_buffer*range where max gate_buffer = 8 and max range = 3
	
	};
	
extern struct arp arp;		 


void step_arp_note(void);
void update_arp_sequence(void);




#endif