#include <avr/io.h>

#include "utils.h"
#include "hardware.h"
#include "tune.h"
#include "switch_map.h"

void refresh_synth(void) {
			
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
				
	if ((switch_states.byte2 >> PROG_WRITE_SW) & 1) //temporary tune button hack
		{ 
				
		switch_states.byte2 ^= (1<<PROG_WRITE_SW); //toggle read switch state

		tune_8ths(VCO1);
		tune_8ths(VCO2);
				
				
		}
	
}