#ifndef SWITCH_MAP_H
#define SWITCH_MAP_H

//define SPI switch bits - NEED TO CHANGE THESE TO BIT POSITIONS 0-7
#define ISW12_SW			0b00100000
#define ISW13_SW			0b01000000
			

//these switch bits are defined asin bit order coming in on SPI bus (same as data line connected to 74HC165)
#define ISW1_SW				2 //VCO1 SAW U14
#define ISW2_SW				1 //VCO1 TRI U14
#define ISW3_SW				0 //VCO1 PULSE U14
#define ISW4_SW				7 //SYNC U14
#define ISW5_SW				4 //VCO2 SAW U14
#define ISW6_SW				5 //VCO2 TRI U14
#define ISW7_SW				6 //VCO2 PULSE U14

//define direct MCU input switch bits
#define ISW8_SW				PF2
#define ISW9_SW				PF3
#define ISW11_SW		    PF4

#endif