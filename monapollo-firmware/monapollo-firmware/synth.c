#include <avr/io.h>
#include <util/delay.h>

#include "utils.h"
#include "hardware.h"
#include "synth.h"
#include "tune.h"
#include "switch_map.h"
#include "display.h"

struct patch patch = {0,0,0,0,0};
	
static struct octave_index octave_index = {0,0};
			
//static uint8_t octave_index = 0;

uint8_t vco1_octave[5] = 
	{
		VCO1_32F,
		VCO1_16F,
		VCO1_8F,
		VCO1_4F,
		VCO1_2F				
	};
	
uint8_t vco2_octave[5] =
	{
		VCO2_32F,
		VCO2_16F,
		VCO2_8F,
		VCO2_4F,
		VCO2_2F
	};		

uint8_t add_octave_to_note (uint8_t note, uint8_t vco) {
	
	uint8_t n = 0;
	
	
	//mask chops off top 4 bits for VCO1, which are octave index for VCO2
	//or mask chops off bottom 4 bits and shift right 4 bits to get octave index for VCO2	
	//n = octave_index & octave_index_mask;
	n = octave_index.vco1;
	if (vco == VCO2) n = octave_index.vco2;

	note = (n*12) + note; //calculate MIDI note after octave addition
	if (note > 136) { //note is beyond range
			
		note = 136;
			
	}
		
	return note;	
	
}

void update_octave_range(void) {
	
	if ((switch_states.byte0 >> VCO1_OCTAVE_UP_SW) & 1) {
		
		if (++octave_index.vco1 == 5) octave_index.vco1 = 4;
		switch_states.byte0 ^= (1<<VCO1_OCTAVE_UP_SW); //toggle switch state bit

		
	}
	
	if ((switch_states.byte1 >> VCO1_OCTAVE_DOWN_SW) & 1) { //this didn't work initially because VCO1_OCTAVE_DOWN_SW pull down resistor wasn't installed on PCB!!!
	
		if (octave_index.vco1 == 0) {} else {octave_index.vco1--;}
		switch_states.byte1 ^= (1<<VCO1_OCTAVE_DOWN_SW);
		//patch.byte_4 &= 0b11100000;
		//patch.byte_4 |= (1<<vco1_octave[octave_index.vco1]);

	}
	
	patch.byte_4 = 0; //clear the whole damn byte as all bits are set below
	//patch.byte_4 &= 0b11100000; //clear vco1 octave bits - this masks top 3 bits which are for VCO2 octave LEDs
	patch.byte_4 |= (1<<vco1_octave[octave_index.vco1]); //set octave	
	
	if ((switch_states.byte1 >> VCO2_OCTAVE_UP_SW) & 1) {
		
		if (++octave_index.vco2 == 5) octave_index.vco2 = 4;
		switch_states.byte1 ^= (1<<VCO2_OCTAVE_UP_SW); //toggle switch state bit		
	}	
	
	if ((switch_states.byte1 >> VCO2_OCTAVE_DOWN_SW) & 1) {
		
		if (octave_index.vco2 == 0) {} else {octave_index.vco2--;}
		switch_states.byte1 ^= (1<<VCO2_OCTAVE_DOWN_SW);
		
	}
	
	//patch.byte_4 &= 0b00011111; //clear vco2 octave bits - this masks bottom 5 bits which are for VCO1 octave LEDs
	
	patch.byte_3 &= 0b11111100; //clear bottom 2 bits for patch byte_3, which are for VCO2 2' and 4'
	
	if (octave_index.vco2 > 2) { //VCO2 2' and 4' LEDs are on LED latch 3

						
		patch.byte_3 |= (1<<vco2_octave[octave_index.vco2]);	
				
	} else { //VCO2 8', 16' and 32' are on LED latch 4
		
		patch.byte_4 |= (1<<vco2_octave[octave_index.vco2]); //set octave
	}	
	
}
	
	
void refresh_synth(void) {
	
	//parse LED data for LED latch 5
	patch.byte_5 =	((switch_states.byte0 >> VCO_SYNC_SW) & 1) << VCO_SYNC |					
					((switch_states.byte0 >> VCO1_SAW_SW) & 1) << VCO1_SAW |
					((switch_states.byte0 >> VCO1_TRI_SW) & 1) << VCO1_TRI |
					((switch_states.byte0 >> VCO1_PULSE_SW) & 1) << VCO1_PULSE |
					((switch_states.byte0 >> VCO2_SAW_SW) & 1) << VCO2_SAW |
					((switch_states.byte0 >> VCO2_TRI_SW) & 1) << VCO2_TRI |
					((switch_states.byte0 >> VCO2_PULSE_SW) & 1) << VCO2_PULSE |
					((switch_states.byte2 >> BMOD_SW) & 1) << BMOD;
	
			
	//update analog switch latch:
	//need to incorporate BMOD_LATCH_BIT switch state into data byte sent to analog switch latch
	//3rd switch bit is VCO1_OCTAVE_UP_SW state, which isn't used by analog switch latch
	uint8_t analog_sw_byte = switch_states.byte0;
	uint8_t BMOD_SW_ON = (switch_states.byte2 >> BMOD_SW) & 1;
	analog_sw_byte ^= (-BMOD_SW_ON ^ analog_sw_byte) & (1<<3);//set third bit dependent on BMOD switch state
	update_analog_switch_latch(analog_sw_byte);
					
	//set EG2 INV bit. This changes the nth bit to x from: http://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit-in-c-c
	//need to make sure this doesn't interfere with anything else on this port
	uint8_t EG2_INV_ON = (switch_states.byte2 >> EG2_INV_SW) & 1;
	EG2_POL_PORT ^= (-EG2_INV_ON ^ EG2_POL_PORT) & (1<<EG2_POL);
	
	//parse octave switch data
	update_octave_range();

	
	//value_to_display = octave_index;			
				
	if ((switch_states.byte2 >> PROG_WRITE_SW) & 1) //temporary tune button hack
		{ 
				
		switch_states.byte2 ^= (1<<PROG_WRITE_SW); //toggle read switch state

		tune_8ths(VCO1);
		tune_8ths(VCO2);
		_delay_ms(200);	//give some time for release to decay to avoid pops	
				
		}
		
		
	
}

