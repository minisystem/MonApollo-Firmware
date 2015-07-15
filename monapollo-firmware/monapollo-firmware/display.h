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

extern volatile uint16_t value_to_display;

void display_dec(uint16_t number, uint8_t digit);

void update_display(uint16_t number, uint8_t type);

#endif