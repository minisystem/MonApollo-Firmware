#include <avr/io.h>
#include "assigner.h"

static struct midi_note note_pool[8] = 
{
	{-1,0},
	{-1,0},
	{-1,0},	
	{-1,0},
	{-1,0},
	{-1,0},
	{-1,0},
	{-1,0}									
};


//static uint8_t free_slot = 0;

uint8_t get_current_note() {
	
	return note_pool[0].note;
	
}

void new_note(uint8_t note, uint8_t velocity) {
	

		
	//shift contents of note_pool right by one element
	memmove(note_pool + 1, note_pool, sizeof(note_pool) -sizeof(*note_pool)); //sizeof struct - sizeof one element of struct
		
	
	
	note_pool[0].note = note;
	note_pool[0].velocity = velocity;
	
	
	
	
} 
void remove_note(uint8_t note){
	
	uint8_t free_slot = 0;
	
	for (int i = 0; i <= 7; i++) { //search for note in note stack
		
		if (note_pool[i].note == note) {
			
			note_pool[i].note = -1;
			free_slot = i;
			break;
			
		}		
		
	}
	
	//if it's the first note, then return. Omar pointed out that this optimization will cause problems if you release most recently played note
	//if (free_slot == 0) return;

	//now shift elements left	
	memmove(note_pool + free_slot, note_pool + free_slot + 1, sizeof(note_pool) - (sizeof(*note_pool)*(free_slot + 1)));
	note_pool[7].note = -1;
	
	//if releasing the last note in the stack then need to preserve pitch for envelope release
	//if (free_slot == 0) note_pool[0].note = note;
	
}



	

	
	
	