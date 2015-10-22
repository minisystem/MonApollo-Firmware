#ifndef ADC_H
#define ADC_H

//define POTMUX_EN bits
#define POTMUX_EN0	PH6
#define POTMUX_EN1	PH7

#define NUM_POTS	29 //number of pots that need to be read and have values stored (doesn't include volume pot)

struct potentiometer {
	
	uint16_t value:10;
	uint8_t locked_value;
	uint8_t channel:4;
	uint8_t mux_addr:3;
	uint8_t locked:1;
	
	
	};
	


extern struct potentiometer vco2_mix_pot;
extern struct potentiometer vco1_mix_pot;
extern struct potentiometer pitch_eg2_pot;
extern struct potentiometer pitch_vco2_pot;
extern struct potentiometer pitch_lfo_pot;
extern struct potentiometer pwm_lfo_pot;
extern struct potentiometer pwm_eg2_pot;
extern struct potentiometer vco1_pw_pot;
extern struct potentiometer fine_pot;
extern struct potentiometer tune_pot;
extern struct potentiometer lfo_rate_pot;
extern struct potentiometer arp_rate_pot;
extern struct potentiometer glide_pot;
extern struct potentiometer amp_lfo_pot;
extern struct potentiometer volume_pot;
extern struct potentiometer vco2_pw_pot;

extern struct potentiometer fil_eg2_pot;
extern struct potentiometer res_pot;
extern struct potentiometer cutoff_pot;
extern struct potentiometer key_track_pot;
extern struct potentiometer fil_vco2_pot;
extern struct potentiometer fil_lfo_pot;
extern struct potentiometer noise_mix_pot;
extern struct potentiometer attack_2_pot;
extern struct potentiometer attack_1_pot;
extern struct potentiometer decay_2_pot;
extern struct potentiometer decay_1_pot;
extern struct potentiometer sustain_2_pot;
extern struct potentiometer sustain_1_pot;
extern struct potentiometer release_2_pot;
extern struct potentiometer release_1_pot;

extern struct potentiometer *pot_id[30];


void setup_adc(void);

//uint16_t read_pot(uint8_t mux_select, uint8_t channel);

uint16_t read_pot(struct potentiometer *pot);

#endif
