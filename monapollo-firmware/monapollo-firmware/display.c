#include <avr/io.h>

#include "display.h"
#include "hardware.h"
#include "display_map.h"



void display_dec(uint16_t number, uint8_t place)
{
	
	uint8_t digit[] = {
		ONES,
		TENS,
		HUNDS,
		THOUS,
	};
	
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
	DATA_BUS = digit[place];
	//latch data to anode lines
	DISPLAY_PORT |= (1<<DISP_ANODE_LATCH);
	DISPLAY_PORT &= ~(1<<DISP_ANODE_LATCH);

	
	//determine cathode byte based on digit to display
	uint8_t cathode_byte;
	
	static uint8_t digit_index[4]; //array to hold digit index
	
	//optimized code to handle numbers from 0 to 999 from http://embeddedgurus.com/stack-overflow/2011/02/efficient-c-tip-13-use-the-modulus-operator-with-caution/
	//would be nice to make this work for numbers from 0 to 9999 but I'll need to understand the optimized divsion routine to make it work from numbers > 999
	static const uint8_t rem[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1};
	uint16_t q1;
	uint8_t q2, q3;
	uint8_t q, r;

	uint8_t msd, nsd, lsd;

	//if (number > 999) {
	//number = 999;
	//}

	/* First division by 10 */

	q1 = (number >> 1) + (number >> 2);
	q1 += q1 >> 4;
	q1 += q1 >> 8;
	q2 = q1 >> 3;
	r = number - q2 * 10;
	q = q2 + (r > 9);
	lsd = rem[r];

	/* Second division by 10 */

	q3 = (q >> 1) + (q >> 2);
	q3 += q3 >> 4;
	q3 = q3 >> 3;
	r = q - q3 * 10;
	msd = q3 + (r > 9);
	nsd = rem[r];

	digit_index[0] = lsd;
	digit_index[1] = nsd;
	digit_index[2] = msd;
	digit_index[3] = 0;

	cathode_byte = dec[digit_index[place]];
	
	//set cathode byte
	DATA_BUS = ~(cathode_byte); //set bits for cathode (current sinks, active LOW)
	//latch data to cathode lines
	DISPLAY_PORT |= (1<<DISP_CATHODE_LATCH);
	DISPLAY_PORT &= ~(1<<DISP_CATHODE_LATCH);
	
	//DATA_BUS = 0; //clear DATA_BUS before return
}

void update_display(uint16_t number, uint8_t type) {
	
	static uint8_t place = 0;
	

	
	if (type == DEC) {

		
			
		display_dec(number, place);
		//increment digit display place
		if (place++ == 3) //post increment
		{
			place = 0;
		}	
		
		
		
	} else { //type is HEX
		
		
	}
	
	
}