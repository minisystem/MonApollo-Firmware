#ifndef UTILS_H
#define UTILS_H

uint8_t read_switch_port(void);
void update_analog_switch_latch(uint8_t switch_state_byte);

struct switch_states {
	
	uint8_t byte0; //first spi byte
	uint8_t byte1; //second spi byte
	uint8_t byte2; //MCU switch port byte
	
	};

extern struct switch_states switch_states;	



#endif