#include <avr/io.h>
#include "assigner.h"
#include "clock.h"
#include "display.h" //added just so display can be used for troubleshooting
#include "arp.h"

static struct midi_note note_pool[8] = //create a pool to store and order incoming MIDI notes. Eventualyl want this to be initialized to NOTE_POOL_SIZE definition in assigner.h
{
	{EMPTY,	0},
	{EMPTY,	0},
	{EMPTY,	0},	
	{EMPTY,	0},
	{EMPTY,	0},
	{EMPTY,	0},
	{EMPTY,	0},
	{EMPTY,	0}									
};

uint8_t gate_buffer = 0;	

static uint8_t current_note = 0; //this acts as a buffer for the current note to maintain pitch during release stage of envelopes

uint8_t get_indexed_note(uint8_t index) {
	
	return note_pool[index].note;
	
	
	
}
uint8_t get_current_note() { //Force inline this function? Ask Omar. BUT, this could be where interpolation is calculated!
	
	uint8_t note = 0;
	if (arp.clock_source == OFF) {
		
		note = current_note;

		
	} else {
		
		//do something here to determine what arp note should be
		note = arp.current_note; //yeah, like this!
		
		
	}
	return note;
	
}

void new_note(uint8_t note, uint8_t velocity) {
	
	
	//shift contents of note_pool right by one element
	memmove(note_pool + 1, note_pool, sizeof(note_pool) -sizeof(*note_pool)); //last argument: sizeof struct - sizeof one element of struct. See http://www.cplusplus.com/forum/beginner/1936/
	
	//add new note to pool
	note_pool[0].note = note;
	note_pool[0].velocity = velocity;
	
	current_note = note; //set current note
	
} 
void remove_note(uint8_t note){
	
	uint8_t free_slot = 0;
	uint8_t temp_note = note_pool[0].note; //holder for current note
	for (int i = 0; i <= 7; i++) { //search for note in note stack
		
		if (note_pool[i].note == note) {
			
			note_pool[i].note = EMPTY;
			free_slot = i;
			break;
			
		}		
		
	}
	
	
	//now shift elements left	
	memmove(note_pool + free_slot, note_pool + free_slot + 1, sizeof(note_pool) - (sizeof(*note_pool)*(free_slot + 1)));
	note_pool[7].note = EMPTY;	
	
	//check to see if this is the last note released		
	//if it is, this might be a good place to ensure gate_buffer is empty
	if (note_pool[0].note == EMPTY) {
		
		current_note = temp_note; //store last note released for maintaining pitch during envelope release stage
		
	} else {		
		
		current_note = note_pool[0].note; //otherwise, the current note is the next one in the note stack

	}	
	
}



	

	
	
	