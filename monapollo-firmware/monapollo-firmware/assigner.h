#ifndef ASSIGNER_H
#define ASSIGNER_H

//

#define EMPTY 255 //flag for empty note
#define NOTE_POOL_SIZE 7 //not used yet

#define MIN_NOTE	8 //minimum MIDI note

struct midi_note {
	
	uint8_t note;
	uint8_t velocity;
	
	};
	
	

 //MIDI gate buffer for note stealing
extern uint8_t gate_buffer;	

uint8_t get_current_note(); //return note at top of note stack
uint8_t get_indexed_note(uint8_t index); //return indexed note in note pool
void new_note(uint8_t note, uint8_t velocity); //add incoming MIDI note to pool. Called when MIDI NOTE ON event occurs
void remove_note(uint8_t note); //remove note from note pool and reorder pool. Called when MIDI NOTE OFF event occurs

#endif