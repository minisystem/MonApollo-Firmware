#ifndef ASSIGNER_H
#define ASSIGNER_H

//

#define EMPTY 255 //flag for empty note
#define NOTE_POOL_SIZE 7

struct midi_note {
	
	uint8_t note;
	uint8_t velocity;
	
	};
	

uint8_t get_current_note(); //return note at top of note stack
void new_note(uint8_t note, uint8_t velocity); //add incoming MIDI note to pool. Called when MIDI NOTE ON event occurs
void remove_note(uint8_t note); //remove note from note pool and reorder pool. Called when MIDI NOTE OFF event occurs

#endif