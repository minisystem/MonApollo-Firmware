#include <avr/io.h>
#include <util/delay.h>

#include "utils.h"
#include "hardware.h"
#include "synth.h"
#include "tune.h"
#include "switch_map.h"
#include "display.h"

struct patch patch = {0,0,0,0,0};
	
static uint8_t octave_index = 0;

uint8_t vco1_octave[5] = 
	{
		{VCO1_32F},
		{VCO1_16F},
		{VCO1_8F},
		{VCO1_4F},
		{VCO1_2F}				
	};
	
uint8_t vco2_octave[5] =
	{
		{VCO2_32F},
		{VCO2_16F},
		{VCO2_8F},
		{VCO2_4F},
		{VCO2_2F}
	};		
	
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
	if ((switch_states.byte0 >> VCO1_OCTAVE_UP_SW) & 1) {
	
		if (++octave_index == 5) octave_index = 4;
		switch_states.byte0 ^= (1<<VCO1_OCTAVE_UP_SW); //toggle switch state bit
		patch.byte_4 &= 0b1110000; //clear vco1 octave bits - this masks top 3 bits which are for VCO2 octave LEDs
		patch.byte_4 |= (1<<vco1_octave[octave_index]); //set octave
		
	}
	
	if ((switch_states.byte1 >> VCO1_OCTAVE_DOWN_SW) & 1) { //this didn't work initially because VCO1_OCTAVE_DOWN_SW pull down resistor wasn't installed on PCB!!!
		
		if (octave_index == 0) {} else {octave_index--;}
		switch_states.byte1 ^= (1<<VCO1_OCTAVE_DOWN_SW);	
		patch.byte_4 &= 0b11100000;
		patch.byte_4 |= (1<<vco1_octave[octave_index]);	
		
	}	
	
	value_to_display = octave_index;			
				
	if ((switch_states.byte2 >> PROG_WRITE_SW) & 1) //temporary tune button hack
		{ 
				
		switch_states.byte2 ^= (1<<PROG_WRITE_SW); //toggle read switch state

		tune_8ths(VCO1);
		tune_8ths(VCO2);
		_delay_ms(200);	//give some time for release to decay to avoid pops	
				
		}
		
		
	
}