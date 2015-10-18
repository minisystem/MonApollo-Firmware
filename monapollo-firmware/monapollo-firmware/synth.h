#ifndef SYNTH_H
#define SYNTH_H

#define NUM_PATCHES 36 //max number of patches

void update_patch(void);
void update_octave_range(void);
uint8_t transpose_note(uint8_t note, uint8_t vco);

void save_patch(uint8_t patch_number);
void load_patch(uint8_t patch_number);
void update_patch_programmer(void);
void update_lfo_shape(void);
void update_lfo_sync(void);

void lock_pots(void);
void unlock_pots(void);

enum patch_mode {
	
	MEMORY, //memory mode, pots locked
	MANUAL, //manul mode, pots unlocked
	EDIT, //edit mode, some pots or switches have changed from memory
	WRITE, //select same or new memory location and write patch to memory
	MIDI, //select new midi channel - show 'CHXX' on display to indicate MIDI channel is being selected. WRITE button exits. Activate this mode by pressing PROG UP/DOWN buttons simultaneously?
	CAL, //not integrated with tuning routine yet,but will be useful flag for running tuning calibration routine
	TEST //some kind of diagnostic mode that might be useful for manually setting VCO init_cvs???
	};
	
extern enum patch_mode mode;	

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
	
	uint8_t number; //patch number
	uint8_t midi_ch; //current MIDI channel
	
	uint8_t arp_clock_source; //clock source: MIDI clock or ARP clock. becomes MIDI clock when sync is active
	uint8_t lfo_clock_source; //
	
	enum patch_mode mode;
	
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

extern uint8_t switch_press; //global flag to detect any switch press except 


struct octave_index {
	
	uint8_t vco1; //could turn into 3 bit long bit field, but memory saved is probably not worth overhead 
	uint8_t vco2;
	
};

struct lfo {
	
	uint8_t waveform_addr;  //could turn into bit field, but memory saved is probably not worth overhead 
	uint8_t led_addr;
	
	};

#endif