#ifndef SWITCH_MAP_H
#define SWITCH_MAP_H

//define SPI switch bits - number corresponds to bit position in read SPI byte

//SPI switch register 1 (5th SPI byte)
#define VCO_SYNC_SW			7 //VCO_SYNC U14
#define VCO2_PULSE_SW		6 //VCO2 PULSE U14
#define VCO2_TRI_SW			5 //VCO2 TRI U14
#define VCO2_SAW_SW			4 //VCO2 SAW U14	
#define VCO1_OCTAVE_UP		3 //VCO1 OCTAVE UP	
#define VCO1_SAW_SW			2 //VCO1 SAW U14
#define VCO1_TRI_SW			1 //VCO1 TRI U14	
#define VCO1_PULSE_SW		0 //VCO1 PULSE U14

//SPI switch register 2 (4th SPI byte)
#define LFO_SHAPE_SW		7
#define ARP_SYNC_SW			6
#define ARP_MODE_SW			5
#define ARP_RANGE_SW		4
#define VCO1_OCTAVE_DOWN_SW	3
#define VCO2_OCTAVE_UP_SW	2
#define VCO2_OCTAVE_DOWN_SW	1
#define LFO_SYNC_SW			0


//define direct MCU input switch bits
#define BMOD_SW					PF2
#define EG2_INV_SW				PF3
#define PROG_WRITE_SW		    PF4
#define PROG_UP_SW				PF6
#define PROG_DOWN_SW			PF5
#define PROG_MANUAL_SW			PF7

#endif