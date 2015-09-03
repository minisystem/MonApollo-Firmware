#ifndef SYNTH_H
#define SYNTH_H

struct patch {
	
	//patch switch/mode data is stored in these bytes
	//data is organized so that each byte can be sent directly out to SPI to set LEDs
	uint8_t byte_1;
	uint8_t byte_2;
	uint8_t byte_3;
	uint8_t byte_4;
	uint8_t byte_5;
	
	};

#endif