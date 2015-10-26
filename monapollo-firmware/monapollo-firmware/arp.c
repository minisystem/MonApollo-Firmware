#include <stdio.h>
#include "arp.h"
#include "assigner.h"
//#include "display.h"

struct arp arp;


//static struct midi_note arp_sequence[24] = {0,0};

void update_arp_sequence(void) {
	
	//if (gate_buffer == 0) {
		//
		//arp.current_note = arp.sequence[arp.last_step].note; //temp note to store for release phase
		//arp.step_position = 0; //reset step position
		////return;
	//}

	arp.step_number = gate_buffer*(arp.range + 1);
	
	
	//if ((arp.mode == UP_DOWN) && (arp.range > 0)) arp.step_number = arp.step_number << 1;//twice as many steps
	
	//set arp sequence
	uint8_t step = 0;
	
	
	for (int range = 0; range <= arp.range; range++) {
			
		for (int i = 0; i < gate_buffer; i++) { // NOTE < rather than <= - <= was causing extra iteration that was over incrementing step!!!!!
			
			uint8_t root_note = get_indexed_note(gate_buffer - i - 1); //need to get notes in reverse order out of note pool
			//uint8_t root_note = get_indexed_note(i);
				
			switch(arp.mode) {
				
				case UP:
				case UP_DOWN:
					arp.sequence[step++].note = root_note + (range*12); //will need to handle max note out of range here
				
					break;
					
				case DOWN:
					
					//int temp_note = int (root_note - (range*12));
					//if (temp_note < 8)  temp_note = root_note; //handle min note out of range here
					arp.sequence[step++].note = root_note - (range*12);
					
				
					break;	
					
					
				case RANDOM:
				
					//what is random? note sequence or octave? It's both
				
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
			//if (next_position >= arp.step_number) {
				//
				//step_change = -1;
				//arp.step_position += step_change;
				//
			//} else if (next_position < 0) {
				//
				//step_change = 1;
				//arp.step_position += step_change;
				//
			//} else {
				//
				//arp.step_position += step_change;
				//
			//}
			
			break;
			
		case RANDOM:
		
			break;
			
		default:
		
			if (++arp.step_position >= arp.step_number) arp.step_position  = 0;//reset step position when at end of sequence 
			break;			
		
		
	}
	 
	
	//arp.current_note = arp.sequence[arp.step_position].note;
	

}