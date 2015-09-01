#ifndef SWITCH_MAP_H
#define SWITCH_MAP_H

//define SPI switch bits - number corresponds to bit position in read SPI byte
//SPI switch register 0 (4th SPI byte)
#define ISW12_SW			5
#define ISW13_SW			6

//SPI switch register 1 (5th SPI byte)			
#define ISW1_SW				2 //VCO1 SAW U14
#define ISW2_SW				1 //VCO1 TRI U14
#define ISW3_SW				0 //VCO1 PULSE U14
#define ISW4_SW				7 //SYNC U14
#define ISW5_SW				4 //VCO2 SAW U14
#define ISW6_SW				5 //VCO2 TRI U14
#define ISW7_SW				6 //VCO2 PULSE U14



//define direct MCU input switch bits
#define BMOD_SW				PF2
#define EG2_INV				PF3
#define PROG_WRITE		    PF4

#endif