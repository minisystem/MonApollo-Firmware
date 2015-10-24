#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#include "adc.h"
#include "utils.h"
#include "hardware.h"
#include "synth.h"
#include "tune.h"
#include "switch_map.h"
#include "spi.h"
#include "display.h"
#include "clock.h"
#include "arp.h"
#include "assigner.h"

struct patch current_patch = {0};
struct eeprom_patch EEMEM patch_memory[NUM_PATCHES]; //EEPROM at 1910 bytes after tuning data is stored. Will still need to save MIDI channel  and a couple of bytes of other data. Currently EEPROM is 93.3% full.	
	
static struct octave_index octave_index = {0,0};
	
	
uint8_t switch_press = 0;

struct lfo lfo[] = 

	{
		{LFO_TRI_ADDR, LFO_TRI},
		{LFO_SINE_ADDR, LFO_SINE}, 
		{LFO_SAW_ADDR, LFO_SAW},
		{LFO_PULSE_ADDR, LFO_RNDM},//no LED for pulse, so just use RNDM LED twice
		{LFO_RNDM_ADDR, LFO_RNDM}		
	
	};	
	
static uint8_t lfo_shape_index = 0;	
			
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
	
uint8_t lfo_shape[5] = 
	{
		LFO_TRI_ADDR,
		LFO_SINE_ADDR,
		LFO_SAW_ADDR,
		LFO_PULSE_ADDR,
		LFO_RNDM_ADDR
	};
	

void lock_pots(void) { //run this every time new patch is loaded to lock pots and store locked values

for (int i = 0; i <= NUM_POTS; i++) {
	
	pot_id[i]->locked_value = (pot_id[i]->value) >> 2;
	pot_id[i]->locked = 1;
	
}

}

void unlock_pots(void) {
	
	for (int i = 0; i <= NUM_POTS; i++) {
		
		pot_id[i]->locked = 0;
		
	}
	
}			
	
	
void save_patch(uint8_t patch_number) {
	
	struct eeprom_patch patch_to_save;
	//because of bit fields in eeprom patch struct, a temporary eeprom patch needs to be filled with current_patch values and then saved to memory.
	
	patch_to_save.vco2_pw = current_patch.vco2_pw;
	patch_to_save.vco1_mix = current_patch.vco1_mix;
	patch_to_save.pitch_eg2 = current_patch.pitch_eg2;
	patch_to_save.pitch_vco2 = current_patch.pitch_vco2;
	patch_to_save.pitch_lfo = current_patch.pitch_lfo;
	patch_to_save.pwm_lfo = current_patch.pwm_lfo;
	patch_to_save.pwm_eg2 = current_patch.pwm_eg2;
	patch_to_save.vco1_pw = current_patch.vco1_pw;
	patch_to_save.fine = current_patch.fine;
	patch_to_save.tune = current_patch.tune; //probably shouldn't save master tune value. Like volume, it is a parameter that doesn't apply to a patch.
	patch_to_save.lfo_rate = current_patch.lfo_rate;
	patch_to_save.arp_rate = current_patch.arp_rate;
	patch_to_save.glide	= current_patch.glide;
	patch_to_save.amp_lfo = current_patch.amp_lfo;
	patch_to_save.vco2_pw = current_patch.vco2_pw;
	
	patch_to_save.fil_eg2 = current_patch.fil_eg2;
	patch_to_save.res = current_patch.res;
	patch_to_save.cutoff = current_patch.cutoff;
	patch_to_save.key_track = current_patch.key_track;
	patch_to_save.fil_vco2 = current_patch.fil_vco2;
	patch_to_save.fil_lfo = current_patch.fil_lfo;
	patch_to_save.noise_mix = current_patch.noise_mix;
	patch_to_save.attack_2 = current_patch.attack_2;
	patch_to_save.attack_1 = current_patch.attack_1;
	patch_to_save.decay_2 = current_patch.decay_2;
	patch_to_save.decay_1 = current_patch.decay_1;
	patch_to_save.sustain_2 = current_patch.sustain_2;
	patch_to_save.sustain_1 = current_patch.sustain_1;
	patch_to_save.release_2 = current_patch.release_2;
	patch_to_save.release_1 = current_patch.release_1;
	
		
	patch_to_save.byte_1 = current_patch.byte_1;
	patch_to_save.byte_2 = current_patch.byte_2;
	patch_to_save.byte_3 = current_patch.byte_3;
	patch_to_save.byte_4 = current_patch.byte_4;
	patch_to_save.byte_5 = current_patch.byte_5;
	
	
	lock_pots();
	//if (current_patch.mode == MANUAL) switch_states.byte2 &= ~(1<< PROG_MANUAL_SW); //if in MANUAL mode, turn off MANUAL LED
	switch_states.byte2 &= ~(1<< PROG_MANUAL_SW);
	current_patch.mode = MEMORY;
	
	eeprom_update_block((const void*)&patch_to_save, (void*)&patch_memory[patch_number], sizeof(patch_to_save));
}	
	
void load_patch(uint8_t patch_number) {
	
	struct eeprom_patch loaded_patch;
	//because of bit fields in eeprom patch struct, a temporary eeprom patch needs to be filled with current_patch values and then saved to memory.
	eeprom_read_block((void*)&loaded_patch, (const void*)&patch_memory[patch_number], sizeof(loaded_patch));
	
	
	
	current_patch.vco2_pw = loaded_patch.vco2_pw;
	current_patch.vco1_mix = loaded_patch.vco1_mix;
	current_patch.pitch_eg2 = loaded_patch.pitch_eg2;
	current_patch.pitch_vco2 = loaded_patch.pitch_vco2;
	current_patch.pitch_lfo = loaded_patch.pitch_lfo;
	current_patch.pwm_lfo = loaded_patch.pwm_lfo;
	current_patch.pwm_eg2 = loaded_patch.pwm_eg2;
	current_patch.vco1_pw = loaded_patch.vco1_pw;
	current_patch.fine = loaded_patch.fine;
	current_patch.tune = loaded_patch.tune; //probably shouldn't save master tune value. Like volume, it is a parameter that doesn't apply to a patch.
	current_patch.lfo_rate = loaded_patch.lfo_rate;
	current_patch.arp_rate = loaded_patch.arp_rate;
	current_patch.glide	= loaded_patch.glide;
	current_patch.amp_lfo = loaded_patch.amp_lfo;
	current_patch.vco2_pw = loaded_patch.vco2_pw;
	
	current_patch.fil_eg2 = loaded_patch.fil_eg2;
	current_patch.res = loaded_patch.res;
	current_patch.cutoff = loaded_patch.cutoff;
	current_patch.key_track = loaded_patch.key_track;
	current_patch.fil_vco2 = loaded_patch.fil_vco2;
	current_patch.fil_lfo = loaded_patch.fil_lfo;
	current_patch.noise_mix = loaded_patch.noise_mix;
	current_patch.attack_2 = loaded_patch.attack_2;
	current_patch.attack_1 = loaded_patch.attack_1;
	current_patch.decay_2 = loaded_patch.decay_2;
	current_patch.decay_1 = loaded_patch.decay_1;
	current_patch.sustain_2 = loaded_patch.sustain_2;
	current_patch.sustain_1 = loaded_patch.sustain_1;
	current_patch.release_2 = loaded_patch.release_2;
	current_patch.release_1 = loaded_patch.release_1;
	
	
	current_patch.byte_1 = loaded_patch.byte_1;
	current_patch.byte_2 = loaded_patch.byte_2;
	current_patch.byte_3 = loaded_patch.byte_3;
	current_patch.byte_4 = loaded_patch.byte_4;
	current_patch.byte_5 = loaded_patch.byte_5;
	
	//using De Bruijn sequence to determine which bit is set. For alphabet size k = 2 (binary - 0 and 1) and n = 3. 2^3 = 8. The minimum number of bits required to represent the 5 octave positions
	uint8_t vco1_lookup[] = {7, 2, 5, 0, 6, 4, 3, 1}; // *modified* De Bruijn lookup table for octave number, see: http://stackoverflow.com/questions/14429661/determine-which-single-bit-in-the-byte-is-set
	//lookup table modified from standard 8 bit De Bruijn sequence to handle non sequential order of octave LEDs in byte_4
	uint8_t vco1_bitfield = current_patch.byte_4 & 0b00011111; //clear top 3 bits, which are used for VCO2 octave lookup - probably don't need to clear these bits
	uint8_t bit_index = ((vco1_bitfield*0x1D) >> 4) & 0x7;	//0x1D 0b11101 is the De Bruijn sequence for 8 bits 
	octave_index.vco1 = vco1_lookup[bit_index];	
	
	uint8_t vco2_lookup[] = {7, 4, 5, 3, 6, 2, 1, 0}; 
																																				 //bit order 4   3    2    1   0
	uint8_t vco2_bitfield = ((current_patch.byte_4 & 0b11100000) >> 3) | (current_patch.byte_3 & 0b00000011); //combine  all VCO2 octave bits into one byte: 8', 16', 32', 4', 2'
	bit_index = ((vco2_bitfield*0x1D) >> 4) & 0x7;																								     //index 2   1    0    3   4
	octave_index.vco2 = vco2_lookup[bit_index];
	
	uint8_t lfo_lookup[] = {0, 0, 2, 2, 1, 3, 3, 1}; //bits 7, 5, 4, 6 are irrelevant here. Complier seems to be reformatting this table???
	uint8_t lfo_bitfield = current_patch.byte_2 & 0b11110000; //shave off 4 LSBs. Really could use 4 bit De Bruijn sequence here
	bit_index = ((lfo_bitfield*0x1D) >> 4) & 0x7;
	lfo_shape_index = lfo_lookup[bit_index];
	//lfo_shape_index = 0; //reset lfo bytes
	//current_patch.byte_2 = (1<<LFO_TRI);
	
	//set toggle switch bits according to patch data
	//probably need to handle previous switch states here, which are in spi.c
	switch_states.byte0 =	((current_patch.byte_5 >> VCO_SYNC) & 1) << VCO_SYNC_SW |
							((current_patch.byte_5 >> VCO1_SAW) & 1) << VCO1_SAW_SW |
							((current_patch.byte_5 >> VCO1_TRI) & 1) << VCO1_TRI_SW |
							((current_patch.byte_5 >> VCO1_PULSE) & 1) << VCO1_PULSE_SW |
							((current_patch.byte_5 >> VCO2_SAW) & 1) << VCO2_SAW_SW |
							((current_patch.byte_5 >> VCO2_TRI) & 1) << VCO2_TRI_SW |
							((current_patch.byte_5 >> VCO2_PULSE) & 1) << VCO2_PULSE_SW;
							
	switch_states.byte2 &= 0b11110011; //preserve PROG switches states, clear BMOD and EG2 states
	
	switch_states.byte2 |=	((current_patch.byte_5 >> BMOD) & 1) << BMOD_SW |
							((current_patch.byte_1 >> EG2_INV) & 1) << EG2_INV_SW;	
							
	//switch_states.byte1 =					
													
	//spi_sw_byte0_current_state = spi_sw_byte0_previous_state = switch_states.byte0;
	//
	//spi_sw_byte1_current_state = spi_sw_byte1_previous_state = switch_states.byte1;						
			
	lock_pots();
	
	//if (current_patch.mode == MANUAL) switch_states.byte2 &= ~(1<< PROG_MANUAL_SW); //if in MANUAL mode, turn off MANUAL LED
	switch_states.byte2 &= ~(1<<PROG_MANUAL_SW);
	
	current_patch.mode = MEMORY;
	
}

	

		

uint8_t transpose_note (uint8_t note, uint8_t vco) {
	
	uint8_t n = 0;
	
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

	}
	
	current_patch.byte_4 = 0; //clear the whole damn byte as all bits are set below
	current_patch.byte_4 |= (1<<vco1_octave[octave_index.vco1]); //set octave	
	
	if ((switch_states.byte1 >> VCO2_OCTAVE_UP_SW) & 1) {
		
		if (++octave_index.vco2 == 5) octave_index.vco2 = 4;
		switch_states.byte1 ^= (1<<VCO2_OCTAVE_UP_SW); //toggle switch state bit		
	}	
	
	if ((switch_states.byte1 >> VCO2_OCTAVE_DOWN_SW) & 1) {
		
		if (octave_index.vco2 == 0) {} else {octave_index.vco2--;}
		switch_states.byte1 ^= (1<<VCO2_OCTAVE_DOWN_SW);
		
	}
			
	current_patch.byte_3 &= 0b11111100; //clear bottom 2 bits for patch byte_3, which are for VCO2 2' and 4'
	
	if (octave_index.vco2 > 2) { //VCO2 2' and 4' LEDs are on LED latch 3

						
		current_patch.byte_3 |= (1<<vco2_octave[octave_index.vco2]);	
				
	} else { //VCO2 8', 16' and 32' are on LED latch 4
		
		current_patch.byte_4 |= (1<<vco2_octave[octave_index.vco2]); //set octave
	}	
	
}





void update_patch_programmer(void) { //maybe best to use a switch/case statement here instead of a series of if/elses?
	
	if (current_patch.mode == CAL) //temporary tune button hack by pressing and holding PROG DOWN switch
	{
		
		
		//TURN OFF LFO OUTPUT
		DATA_BUS = 0b00000111; //turn off LFO waveform
		LFO_LATCH_PORT |= (1<<LFO_SW_LATCH);
		LFO_LATCH_PORT &= ~(1<<LFO_SW_LATCH);
		DATA_BUS = 0;
		
		tune();
		
		DATA_BUS = LFO_TRI_ADDR;
		LFO_LATCH_PORT |= (1<<LFO_SW_LATCH);
		LFO_LATCH_PORT &= ~(1<<LFO_SW_LATCH);
		DATA_BUS = 0;
		current_patch.byte_2 &= 0b00001111; //clear top 4 bits
		current_patch.byte_2 |= (1<<LFO_TRI);
		
		setup_system_clock(); //restore system clock settings

		current_patch.mode = MEMORY;
		
		return;
		
	}
	

	
	if ((switch_states.byte2>> PROG_UP_SW) & 1) {
		
		switch_states.byte2 ^= (1<<PROG_UP_SW); //toggle switch state bit
		
		if (++current_patch.number == NUM_PATCHES + 1) {			
			
			current_patch.number = NUM_PATCHES; //max patch number
		
					
		} else if (current_patch.mode != WRITE) { //load next patch if not in WRITE mode
			
			load_patch(current_patch.number);
			update_patch();
			
			
		}		
		
	}
	
	if ((switch_states.byte2 >> PROG_DOWN_SW) & 1) {
	
		switch_states.byte2 ^= (1<<PROG_DOWN_SW); //toggle switch state bit

		if (current_patch.number == 1) {} else {current_patch.number--;}
		
		if (current_patch.mode != WRITE) {
			
			load_patch(current_patch.number); 
			update_patch();
		}			
	}
	
	
	if ((switch_states.byte2 >> PROG_WRITE_SW) & 1) {
		
		switch_states.byte2 ^= (1<<PROG_WRITE_SW);
		if (current_patch.mode != WRITE) {
			switch_states.byte2 &= ~(1<< PROG_MANUAL_SW); //turn off manual mode
			current_patch.mode = WRITE;
			lock_pots();
			
			
		} else {
			
			save_patch(current_patch.number); //write the patch
			//switch_states.byte2 &= ~(1<<PROG_WRITE_SW);
		}			
		
		//switch_states.byte2 ^= (1<<PROG_WRITE_SW); //toggle switch state bit
		//save_patch(current_patch.number);
		
		
	}
	
	if (((switch_states.byte2 >> PROG_MANUAL_SW) & 1)) {
			
		switch_states.byte2 |= (1<< PROG_MANUAL_SW);
		current_patch.mode = MANUAL;
		unlock_pots();
			
	}
	//value_to_display = current_patch.number;	
	//value_to_display = vco1_init_cv>>1;
	
	
}	
	
void update_lfo_shape(void) {

	if ((switch_states.byte1 >> LFO_SHAPE_SW) & 1) {
	
		switch_states.byte1 ^= (1<<LFO_SHAPE_SW); //toggle switch state
		if (++lfo_shape_index == 5) lfo_shape_index = 0;
	}
	
	current_patch.byte_2 &= 0b00001111; //clear top 4 bits
	current_patch.byte_2 |= 1 << lfo[lfo_shape_index].led_addr;
	DATA_BUS = lfo[lfo_shape_index].waveform_addr;
	LFO_LATCH_PORT |= (1<<LFO_SW_LATCH);
	LFO_LATCH_PORT &= ~(1<<LFO_SW_LATCH);
	
}		
	
	
void update_lfo_sync(void) {
	
	static uint8_t lfo_sync_mode = 0;
	
	if ((switch_states.byte1 >> LFO_SYNC_SW) & 1) {
			
		switch_states.byte1 ^= (1<<LFO_SYNC_SW); //toggle switch state
		if (++lfo_sync_mode == 5) lfo_sync_mode = 0;
		midi_clock.ppqn_counter = 0; //reset counter
	}
	
	current_patch.byte_2 &= 0b11110000; //clear bottom 4 bits
	if (lfo_sync_mode) current_patch.byte_2 |= (1<<(lfo_sync_mode -1)); //this allows an off state when lfo_sync_mode = 0;
	
	//now parse out clock divide from patch byte_2
	switch (current_patch.byte_2 & 0b00001111) {
		
		case 0b0001:
			midi_clock.divider = 0; //key sync mode - need to turn midi sync off here. How?
			
			break;
			
		case 0b0010: //turn these case conditions into constants. #define 0b0010 DIV_24
			midi_clock.divider = 24; //1:4
			break;
			
		case 0b0100:
			midi_clock.divider = 12; //1:8
			break;
			
		case 0b1000:
			midi_clock.divider = 6; //1:16	 		
			break;
		
		default:
			midi_clock.divider = 0; //need to turn midi sync off here. How?
	}
	
	
	
}

void update_arp_sync(void) {
	
	static uint8_t arp_sync_mode = 0;
	
	if ((switch_states.byte1 >> ARP_SYNC_SW) & 1) {
		
		switch_states.byte1 ^= (1<<ARP_SYNC_SW); //toggle switch state
		if (++arp_sync_mode == 5) arp_sync_mode = 0;
		system_clock.ppqn_counter = 0; //reset counter
	}
	
	current_patch.byte_3 &= 0b11000011; //clear middle 4 bits
	if (arp_sync_mode) current_patch.byte_3 |= 1<<(arp_sync_mode + 1); //this allows an off state when arp_sync_mode = 0. Is that what's really needed?
	
	switch (current_patch.byte_3 & 0b00111100) {
			
		case 0b00000100:
			system_clock.divider = 48; //1:2
			break;
			
		case 0b00001000:
			system_clock.divider = 24; //1:4
			break;
			
		case 0b00010000:		
			system_clock.divider = 12; //1:8
			break;
			
		case 0b00100000:
			system_clock.divider = 6; //1:16	
			break;
			
		default:
			system_clock.divider = 2; //should be 1 but this never turns arp_sync_led off - need to fix this	
				
		}
	
	
	
	
}

void update_arp_range(void) {
	
	static uint8_t arp_range = 0;
	
	if ((switch_states.byte1 == (1<<ARP_RANGE_SW)) & 1) {
		
		switch_states.byte1 ^= (1<<ARP_RANGE_SW); //toggle switch bit
		
		if (++arp_range == 4) arp_range = 0;
		arp.step_position = 0; //reset step position if range changes
						
	}
	
	//arp range LEDs 3 and 2 are in byte_3, bits 7 and 6, respectively. arp range LED 1 is bit 1 of byte_1
	
	current_patch.byte_1 &= ~(1<<ARP_RANGE_1); //clear arp range 1 LED
	current_patch.byte_3 &= 0b00111111; //clear bits 6 and7, arp range 3 and 2
	
	arp.range = arp_range;
	
	
	switch (arp_range) { //this just updates LEDs. no struct to handle arp range yet
		
		case 0:
			
			//set arp range to 0. Haven't designed arp struct to handle this yet
			
			break;
			
		case 1:
		
			current_patch.byte_1 |= (1<<ARP_RANGE_1); //set range 1 LED. LED was in the wrong way!
			break;
			
		case 2:
		
			current_patch.byte_3 |= (1<<ARP_RANGE_2);		
			break;
			
			
		case 3: 
			
			current_patch.byte_3 |= (1<<ARP_RANGE_3);
			break;		
		
	} 
	
	
}


void update_arp_mode(void) {

	static uint8_t arp_mode = 0;
	
	if ((switch_states.byte1 >> ARP_MODE_SW) & 1) {
		
		switch_states.byte1 ^= (1<<ARP_MODE_SW); //toggle switch state
		if (++arp_mode == 5) arp_mode = 0;
		arp.step_position = 0; //reset step position if mode changes
		
	}

	arp.clock_source = INTERNAL_CLOCK;	

	current_patch.byte_1 &= 0b11000011; //clear middle 4 bits UP, DOWN, RANDOM, MODE correspond to bits 6>>2
	
	switch(arp_mode) {
		
		case 0:
		
			//turn arp off
			arp.clock_source = OFF;
			if (gate_buffer == 0) PORTF &= ~(1<<GATE); //turn gate off.
			break;
			
		case 1:
		
			current_patch.byte_1 |= (1<<ARP_MODE_UP) | (1<<ARP_ON);
			arp.mode = UP;
			break;	
		
		case 2:
			current_patch.byte_1 |= (1<<ARP_MODE_DN) | (1<<ARP_ON);
			arp.mode = DOWN;
			break;
			
		case 3:
			current_patch.byte_1 |= (1<<ARP_MODE_UP) | (1<<ARP_MODE_DN) | (1<<ARP_ON);
			arp.mode = UP_DOWN;
			break;
			
		default:
			current_patch.byte_1 |= (1<<ARP_MODE_RD) | (1<<ARP_ON);
			arp.mode = RANDOM;
			break;		
			
					
		
	}
	



}




			
	
void update_patch(void) {
	
	//parse LED data for LED latch 5
	current_patch.byte_5 =	((switch_states.byte0 >> VCO_SYNC_SW) & 1) << VCO_SYNC |					
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
	current_patch.byte_1 ^= (-EG2_INV_ON ^ current_patch.byte_1) & (1 << EG2_INV); //don't forget to set it in patch or it won't be saved!
	
	//parse octave switch data
	update_octave_range();
	
	//parse LFO data
	update_lfo_shape();
	
	//parse LFO sync data
	update_lfo_sync();

	//update arp settings
	update_arp_sync();
	update_arp_range();
	update_arp_mode();
	
	
	//update_patch_programmer();		
				

		
	
}

