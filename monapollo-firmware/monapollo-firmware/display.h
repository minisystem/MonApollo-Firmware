#ifndef DISPLAY_DEFS_H
#define DISPLAY_DEFS_H

#include "display_map.h"
#include "hardware.h"

//LED 7-segment display latches
#define DISP_ANODE_LATCH	PH5
#define DISP_CATHODE_LATCH	PH4

//types of value to display
#define DEC 0
#define HEX 1

enum display_mode {
	
	DECIMAL,
	HEXADECIMAL,
	DASH,
	FLASH
	
	};
	
struct display {
	
	uint16_t value;
	uint8_t digit;
	enum display_mode mode;
	
	
	};	

extern volatile uint16_t value_to_display;

void display_dec(uint16_t number, uint8_t place);

void update_display(uint16_t number, uint8_t type);

#endif