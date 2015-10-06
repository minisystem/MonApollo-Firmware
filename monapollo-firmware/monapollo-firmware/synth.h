#ifndef SYNTH_H
#define SYNTH_H

void refresh_synth(void);
void update_octave_range(void);
uint8_t transpose_note(uint8_t note, uint8_t vco);

void save_patch(uint8_t patch_number);

void set_memory_mode(void);

enum patch_mode {
	
	MEMORY,
	MANUAL,
	EDIT
	
	};

struct patch {
	
	uint16_t vco2_mix;
	uint16_t vco1_mix;
	uint16_t pitch_eg2;
	uint16_t pitch_vco2;
	uint16_t pitch_lfo;
	uint16_t pwm_lfo;
	uint16_t pwm_eg2;
	uint16_t vco1_pw;
	uint16_t fine;
	uint16_t tune;
	uint16_t lfo_rate;
	uint16_t arp_rate;
	uint16_t glide;
	uint16_t amp_lfo;
	uint16_t vco2_pw;
	
	uint16_t fil_eg2;
	uint16_t res;
	uint16_t cutoff;
	uint16_t key_track;
	uint16_t fil_vco2;
	uint16_t fil_lfo;
	uint16_t noise_mix;
	uint16_t attack_2;
	uint16_t attack_1;
	uint16_t decay_2;
	uint16_t decay_1;
	uint16_t sustain_2;
	uint16_t sustain_1;
	uint16_t release_2;
	uint16_t release_1;
	
	//patch switch/mode data is stored in these bytes
	//data is organized so that each byte can be sent directly out to SPI to set LEDs
	uint8_t byte_1; //ARP MODE, ARP RANGE, PROGRAM, EG2 INVERSE
	uint8_t byte_2; //LFO SHAPE + LFO SYNC
	uint8_t byte_3; //ARP RANGE, ARP SYNC, VCO1 4' AND 2' OCTAVE SELECTION
	uint8_t byte_4; //VCO1 + VCO2 OCTAVE SELECTION
	uint8_t byte_5; //VCO WAVEFORMS, VCO SYNC + BMOD
	
	};
	
extern struct patch current_patch;	

struct eeprom_patch { //same as patch but using bit fields to compact memory for writing to EEPROM
	
	uint16_t vco2_mix:10;
	uint16_t vco1_mix:10;
	uint16_t pitch_eg2:10;
	uint16_t pitch_vco2:10;
	uint16_t pitch_lfo:10;
	uint16_t pwm_lfo:10;
	uint16_t pwm_eg2:10;
	uint16_t vco1_pw:10;
	uint16_t fine:10;
	uint16_t tune:10;
	uint16_t lfo_rate:10;
	uint16_t arp_rate:10;
	uint16_t glide:10;
	uint16_t amp_lfo:10;
	uint16_t vco2_pw:10;
	
	uint16_t fil_eg2:10;
	uint16_t res:10;
	uint16_t cutoff:10;
	uint16_t key_track:10;
	uint16_t fil_vco2:10;
	uint16_t fil_lfo:10;
	uint16_t noise_mix:10;
	uint16_t attack_2:10;
	uint16_t attack_1:10;
	uint16_t decay_2:10;
	uint16_t decay_1:10;
	uint16_t sustain_2:10;
	uint16_t sustain_1:10;
	uint16_t release_2:10;
	uint16_t release_1:10;
	
	//patch switch/mode data is stored in these bytes
	//data is organized so that each byte can be sent directly out to SPI to set LEDs
	uint8_t byte_1; //ARP MODE, ARP RANGE, PROGRAM, EG2 INVERSE
	uint8_t byte_2; //LFO SHAPE + LFO SYNC
	uint8_t byte_3; //ARP RANGE, ARP SYNC, VCO1 4' AND 2' OCTAVE SELECTION
	uint8_t byte_4; //VCO1 + VCO2 OCTAVE SELECTION
	uint8_t byte_5; //VCO WAVEFORMS, VCO SYNC + BMOD
	
};


struct octave_index {
	
	uint8_t vco1:3;
	uint8_t vco2:3;
	
};

struct lfo {
	
	uint8_t waveform_addr:3;
	uint8_t led_addr:3;
	
	};

#endif