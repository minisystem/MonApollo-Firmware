#include <avr/io.h>


#include "display.h"
#include "hardware.h"
#include "display_map.h"
#include "synth.h"



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
	//added extra division by 10 to handle 0-9999
	static const uint8_t rem[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1};
	uint16_t q1, q2, qa;
	uint8_t q3, q4, qb;
	uint8_t r;

	uint8_t thous_place, hunds_place, tens_place, ones_place;

	//if (number > 999) {
	//number = 999;
	//}
	
	/* First division by 10 */
	q1 = (number >> 1) + (number >> 2);
	q1 += q1 >> 4;
	q1 += q1 >> 8;
	q2 = q1 >> 3;
	r = number - q2 * 10;
	//r = number - (((q2 << 2) + q2) << 1);
	qa = q2 + (r > 9);
	ones_place = rem[r];

	/* second division by 10 */

	q2 = (qa >> 1) + (qa >> 2);
	q2 += q2 >> 4;
	q2 += q2 >> 8;
	q3 = q2 >> 3;
	r = qa - q3 * 10;
	qb = q3 + (r > 9);
	tens_place = rem[r];

	/* third division by 10 */

	q4 = (qb >> 1) + (qb >> 2);
	q4 += q4 >> 4;
	q4 = q4 >> 3;
	r = qb - q4 * 10;
	thous_place = q4 + (r > 9);
	hunds_place = rem[r];

	digit_index[0] = ones_place;
	digit_index[1] = tens_place;
	digit_index[2] = hunds_place;
	digit_index[3] = thous_place;

	cathode_byte = dec[digit_index[place]];
	
	if (current_patch.mode == EDIT) cathode_byte |= dp;
	if (current_patch.mode == MANUAL) cathode_byte = g; //Roland style dash for MANUAL mode. Could move this to the top as manual mode precludes the need to parse digit
	
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