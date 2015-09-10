#ifndef SYNTH_H
#define SYNTH_H

void refresh_synth(void);
void update_octave_range(void);
uint8_t add_octave_to_note(uint8_t note, uint8_t vco);

struct patch {
	
	//patch switch/mode data is stored in these bytes
	//data is organized so that each byte can be sent directly out to SPI to set LEDs
	uint8_t byte_1; //ARP MODE, ARP RANGE, PROGRAM, EG2 INVERSE 
	uint8_t byte_2; //LFO SHAPE + LFO SYNC
	uint8_t byte_3; //ARP RANGE, ARP SYNC, VCO1 4' AND 2' OCTAVE SELECTION
	uint8_t byte_4; //VCO1 + VCO2 OCTAVE SELECTION
	uint8_t byte_5; //VCO WAVEFORMS, VCO SYNC + BMOD
	
	};
	
extern struct patch patch;

struct octave_index {
	
	uint8_t vco1:3;
	uint8_t vco2:3;
	
};

#endif