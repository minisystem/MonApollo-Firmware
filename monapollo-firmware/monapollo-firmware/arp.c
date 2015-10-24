#include <stdio.h>
#include "arp.h"
#include "assigner.h"

struct arp arp;

//static struct midi_note arp_sequence[24] = {0,0};

void update_arp_sequence(void) {
	
	arp.step_number = gate_buffer + arp.range * gate_buffer; //gate buffer is the number of currently held notes
	
	//set root notes
	for (int step = 0; step < gate_buffer; step++) {
		
		
		for (int range = 0; range < arp.range; range++) {
			
			uint8_t root_note = get_indexed_note(step);
			arp.sequence[step + range].note = root_note;
			
		}
		
	}
	
	switch (arp.mode) {
		
		case UP:
		
		
		
		break;
		
		
		case DOWN:
		
		break;
		
		case UP_DOWN:
		
		break;
		
		case RANDOM:
		
		break;
		
		
		
		
	}
		
	
	
}

void step_arp_note(void) { //updates arp note according to range, mode and keys held
	
	
	if (++arp.step_position > arp.step_number) arp.step_position  = 0; //reset step position when at end of sequence 
	
	arp.current_note = arp.sequence[arp.step_position].note;
	

}