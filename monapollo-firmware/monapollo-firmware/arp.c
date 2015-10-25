#include <stdio.h>
#include "arp.h"
#include "assigner.h"

struct arp arp;


//static struct midi_note arp_sequence[24] = {0,0};

void update_arp_sequence(void) {
	
	//arp.step_number = gate_buffer + arp.range * gate_buffer; //gate buffer is the number of currently held notes
	//uint8_t temp_range = 1;
	arp.step_number = gate_buffer*(arp.range + 1);
	
	
	//set arp sequence
	uint8_t step = 0;
	
	
	for (int range = 0; range <= arp.range; range++) {
		
		
		
		for (int i = 0; i <= gate_buffer; i++) {
			
			uint8_t root_note = get_indexed_note(gate_buffer - i - 1); //need to get notes in reverse order out of note pool
			//uint8_t root_note = get_indexed_note(i);
				
			switch(arp.mode) {
				
				case UP:
				
					arp.sequence[step++].note = root_note + (range*12); //will need to handle max note out of range here
				
					break;
					
				case DOWN:
				
					arp.sequence[step++].note = root_note - (range*12); //will need to handle min note out of range here
				
					break;	
					
				case UP_DOWN:
				
					//not sure what to do here. How does UP/DOWN work. Are there twice as many steps?
				
					break;
					
					
				case RANDOM:
				
					//what is random? note sequence or octave?
				
					break;		
				
			}
			
			
		}
		
	}
	

		
	
	
}

void step_arp_note(void) { //updates arp note according to range, mode and keys held
	
	
	if (++arp.step_position >= arp.step_number) arp.step_position  = 0; //reset step position when at end of sequence 
	
	arp.current_note = arp.sequence[arp.step_position].note;
	

}