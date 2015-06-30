#ifndef LED_MAP_H
#define LED_MAP_H

//define LED bits - NEED TO CHANGE THESE TO BIT POSITIONS 0-7
#define ISW12_LED			0b00000100
#define ISW11_LED			0b10000000
#define ISW9_LED			0 //EG2 INV

#define ISW8_LED			7 //B MOD
#define ISW4_LED			1 //SYNC U16
#define ISW1_LED			2 //VCO1 SAW U16
#define ISW2_LED			3 //VCO1 TRI U16
#define ISW3_LED			0 //VCO1 PULSE U16
#define ISW5_LED			4 //VCO2 SAW U16
#define ISW6_LED			5 //VCO2 TRI U16
#define ISW7_LED			6 //VCO2 PULSE U16

//ARP_SYNC LED driven directly from AVR
#define ARP_SYNC_LED	PB7

#endif