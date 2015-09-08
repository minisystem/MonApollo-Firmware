#include <avr/io.h>

#include "utils.h"
#include "spi.h"
#include "hardware.h"
#include "switch_map.h"

struct switch_states switch_states = {0,0,0};

uint8_t read_switch_port(void) {
	
	static uint8_t previous_port_state = 0;
	
	uint8_t current_port_state = SWITCH_PORT; //read switch state byte
	
	//this toggle code works, but I haven't figured out how it works
	//source: http://forum.allaboutcircuits.com/threads/help-with-programming-uc-toggle-led-using-one-switch.51602/	
	current_port_state ^= previous_port_state;
	previous_port_state ^= current_port_state;
	current_port_state &= previous_port_state;
	
	return current_port_state;
	
}

void update_analog_switch_latch(uint8_t switch_state_byte) {
	
	VCO_SW_LATCH_PORT &= ~(1<<VCO_SW_LATCH);
	//enable output on VCO analog switch latch:
	//switch latch: 7: B TRI 6: B SAW 5: B PULSE 4: B MOD 3: VCO_SYNC_LATCH_BIT 2: A TRI 1: A PULSE 0: A SAW
	DATA_BUS = //I wonder if some kind of bitmask could be implemented here to do a single cycle manipulation rather than a bit-by-bit change
	((switch_state_byte >> VCO_SYNC_SW) & 1) << VCO_SYNC_LATCH_BIT |
	((switch_state_byte >> VCO1_SAW_SW) & 1) << VCO1_SAW_LATCH_BIT |
	((switch_state_byte >> VCO1_TRI_SW) & 1) << VCO1_TRI_LATCH_BIT |
	((switch_state_byte >> VCO1_PULSE_SW) & 1) << VCO1_PULSE_LATCH_BIT |
	((switch_state_byte >> VCO2_SAW_SW) & 1) << VCO2_SAW_LATCH_BIT |
	((switch_state_byte >> VCO2_TRI_SW) & 1) << VCO2_TRI_LATCH_BIT |
	((switch_state_byte >> VCO2_PULSE_SW) & 1) << VCO2_PULSE_LATCH_BIT |
	//BMOD_SW_ON << BMOD_LATCH_BIT;
	((switch_state_byte >> 3) & 1) << BMOD_LATCH_BIT;
	VCO_SW_LATCH_PORT |= (1<<VCO_SW_LATCH);
	
	VCO_SW_LATCH_PORT &= ~(1<<VCO_SW_LATCH);
	DATA_BUS = 0;
	
	
}