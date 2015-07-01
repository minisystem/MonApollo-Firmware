#include <avr/io.h>

#include "display.h"
#include "hardware.h"
#include "display_map.h"

void display_dec(uint16_t number, uint8_t digit)
{
	uint8_t dec[] = {
		//this is the 7-segment display encoding table
		
		ZERO,
		ONE,
		TWO,
		THREE,
		FOUR,
		FIVE,
		SIX,
		SEVEN,
		EIGHT,
		NINE,
		
		
	};
	
	//clear cathode bits
	DATA_BUS = 0xFF; //set bits for cathode (current sinks, active LOW)
	//latch data to cathode lines
	DISPLAY_PORT |= (1<<DISP_CATHODE_LATCH);
	DISPLAY_PORT &= ~(1<<DISP_CATHODE_LATCH);
	
	//set anode bit
	DATA_BUS = digit;
	//latch data to anode lines
	DISPLAY_PORT |= (1<<DISP_ANODE_LATCH);
	DISPLAY_PORT &= ~(1<<DISP_ANODE_LATCH);

	
	//determine cathode byte based on digit to display
	uint8_t cathode_byte;
	
	switch(digit) {
		
		case ONES:
		cathode_byte = dec[(number % 10)]; //print first decimal digit
		break;
		
		case TENS:
		cathode_byte = dec[((number % 100) / 10)]; //print second decimal digit
		break;
		
		case HUNDS:
		cathode_byte = dec[((number % 1000) / 100)]; //print third decimal digit
		break;
		
		case THOUS:
		cathode_byte = dec[((number % 10000) / 1000)]; //print fourth decimal digit
		break;
		
	}
	
	//set cathode byte
	DATA_BUS = ~(cathode_byte); //set bits for cathode (current sinks, active LOW)
	//latch data to cathode lines
	DISPLAY_PORT |= (1<<DISP_CATHODE_LATCH);
	DISPLAY_PORT &= ~(1<<DISP_CATHODE_LATCH);
	
	//DATA_BUS = 0; //clear DATA_BUS before return
}