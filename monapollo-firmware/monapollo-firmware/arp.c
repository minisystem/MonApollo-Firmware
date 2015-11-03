#include <stdio.h>
#include "arp.h"
#include "assigner.h"

//#include "display.h"

struct arp arp;


void update_arp_sequence(void) {
	


	arp.step_number = gate_buffer*(arp.range + 1);
	//arp.step_position = 0; //would use this if note retrigger feature was added
	
	uint8_t step = 0;
	
	//set arp sequence
	for (int range = 0; range <= arp.range; range++) {
			
		for (int i = 0; i < gate_buffer; i++) { // NOTE < rather than <= - <= was causing extra iteration that was over incrementing step!!!!!
			
			uint8_t root_note = get_indexed_note(gate_buffer - i - 1); //need to get notes in reverse order out of note pool
			//uint8_t root_note = get_indexed_note(i);
				
			switch(arp.mode) {
				
				case UP:
				case UP_DOWN:
				case RANDOM:
					arp.sequence[step++].note = root_note + (range*12); //will need to handle max note out of range here
				
					break;
					
				case DOWN:
					
					 //handle min note out of range here
					while (((int)root_note - (range*12)) < MIN_NOTE) { //tested
						
						root_note += 12;
						
					}
					arp.sequence[step++].note = root_note - (range*12);
					
				
					break;	
						
				
			}
			
		}
		//value_to_display = step;

	}
	
	
}

void step_arp_note(void) { //updates arp note according to step position in sequence
	
	arp.current_note = arp.sequence[arp.step_position].note;
	arp.previous_note = arp.current_note;
	
	switch (arp.mode) {
		
		case UP_DOWN:
			
			if (arp.direction == UP) {
				arp.step_position++;	
				
				if (arp.step_position >= arp.step_number) {
					arp.step_position = arp.step_number == 1 ? 0 : arp.step_number - 2; //Omar handles edge case here
					arp.direction = DOWN;
				}				
				
			} else {
				if (arp.step_position <= 0) {
					
					arp.step_position = arp.step_number == 1 ? 0 : 1; //Omar handles edge case here
					arp.direction = UP;
					
				} else {
					arp.step_position--;
				}			
				
			}
			
			break;
			
		case RANDOM:
		
			arp.step_position = random() % arp.step_number; //maybe >> random() result to generate 8 bit value before modulo?
		
			break;
			
		default:
		
			if (++arp.step_position >= arp.step_number) arp.step_position  = 0;//reset step position when at end of sequence 
			break;			
		
		
	}
	 
	//arp.current_note = arp.sequence[arp.step_position].note;

}