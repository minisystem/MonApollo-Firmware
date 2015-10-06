#define F_CPU 20000000UL
#include <avr/io.h>
#include <util/delay.h>

#include "adc.h"
#include "hardware.h"


struct potentiometer vco2_mix_pot		={0,0,0,POTMUX_EN0,0};
struct potentiometer vco1_mix_pot		={0,0,1,POTMUX_EN0,0};	
struct potentiometer pitch_eg2_pot		={0,0,2,POTMUX_EN0,0};
struct potentiometer pitch_vco2_pot		={0,0,3,POTMUX_EN0,0};
struct potentiometer pitch_lfo_pot		={0,0,4,POTMUX_EN0,0};
struct potentiometer pwm_lfo_pot		={0,0,5,POTMUX_EN0,0};
struct potentiometer pwm_eg2_pot		={0,0,6,POTMUX_EN0,0};
struct potentiometer vco1_pw_pot		={0,0,7,POTMUX_EN0,0};
struct potentiometer fine_pot			={0,0,8,POTMUX_EN0,0};
struct potentiometer tune_pot			={0,0,9,POTMUX_EN0,0};
struct potentiometer lfo_rate_pot		={0,0,10,POTMUX_EN0,0};
struct potentiometer arp_rate_pot		={0,0,11,POTMUX_EN0,0};
struct potentiometer glide_pot			={0,0,12,POTMUX_EN0,0};
struct potentiometer amp_lfo_pot		={0,0,13,POTMUX_EN0,0};
struct potentiometer volume_pot			={0,0,14,POTMUX_EN0,0};
struct potentiometer vco2_pw_pot		={0,0,15,POTMUX_EN0,0};
	
struct potentiometer fil_eg2_pot		={0,0,1,POTMUX_EN1,0};
struct potentiometer res_pot			={0,0,2,POTMUX_EN1,0};
struct potentiometer cutoff_pot			={0,0,3,POTMUX_EN1,0};
struct potentiometer key_track_pot		={0,0,4,POTMUX_EN1,0};
struct potentiometer fil_vco2_pot		={0,0,5,POTMUX_EN1,0};
struct potentiometer fil_lfo_pot		={0,0,6,POTMUX_EN1,0};
struct potentiometer noise_mix_pot		={0,0,7,POTMUX_EN1,0};
struct potentiometer attack_2_pot		={0,0,8,POTMUX_EN1,0};
struct potentiometer attack_1_pot		={0,0,9,POTMUX_EN1,0};
struct potentiometer decay_2_pot		={0,0,10,POTMUX_EN1,0};
struct potentiometer decay_1_pot		={0,0,11,POTMUX_EN1,0};
struct potentiometer sustain_2_pot		={0,0,12,POTMUX_EN1,0};
struct potentiometer sustain_1_pot		={0,0,13,POTMUX_EN1,0};
struct potentiometer release_2_pot		={0,0,14,POTMUX_EN1,0};
struct potentiometer release_1_pot		={0,0,15,POTMUX_EN1,0};
	
struct potentiometer *pot_id[30] = {

	&vco2_mix_pot,
	&vco1_mix_pot,
	&pitch_eg2_pot,
	&pitch_vco2_pot,
	&pitch_lfo_pot,
	&pwm_lfo_pot,
	&pwm_eg2_pot,
	&vco1_pw_pot,
	&fine_pot,
	&tune_pot,
	&lfo_rate_pot,
	&arp_rate_pot,
	&glide_pot,
	&amp_lfo_pot,
	//&volume_pot, //need to get rid of volume pot and handle it seperately as it is not mapped to patch parameter
	&vco2_pw_pot,
	
	&fil_eg2_pot,
	&res_pot,
	&cutoff_pot,
	&key_track_pot,
	&fil_vco2_pot,
	&fil_lfo_pot,
	&noise_mix_pot,
	&attack_2_pot,
	&attack_1_pot,
	&decay_2_pot,
	&decay_1_pot,
	&sustain_2_pot,
	&sustain_1_pot,
	&release_2_pot,
	&release_1_pot

};	

void setup_adc(void)
{
	//ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0); //set ADC clock to 156.25 KHz for 20 MHz clock
	//ADCSRA |= (1<<ADPS2) | (1<<ADPS1); //set ADC clock to 312.5 KHz for 20 MHz clock
	ADCSRA |= (1<<ADPS2);// | (1<<ADPS0); //set ADC clock to 1.25 MHz for 20 MHz clock
	ADMUX |= (1<<REFS0); //set ADC reference to AVCC (+5V)
	
	DIDR0 |= 0x01; //disable digital input buffer for ADC0
	
	ADCSRA |= (1<<ADEN); //enable ADC
}

uint16_t read_pot(struct potentiometer *pot) {
	
	DATA_BUS = pot->channel;
	POT_MUX &= ~(1<<pot->mux_addr);
	_delay_us(2); //ADC settling time. Previously used 10 us, testing 2 us now. Now testing 1 us. See how it sounds. Nope. Needs to 2 us minimum to prevent crosstalk between multiplexer channels
	ADCSRA |= (1<<ADSC); //start ADC conversion
	while ((ADCSRA & (1<<ADSC))); //wait for ADC conversion to complete (13 cycles of ADC clock - 10.4 us for 1.25Mhz clock) - need to figure out what to do with this time - would interrupt be more efficient?
	POT_MUX |= (1<<pot->mux_addr); //disable pot multiplexer
	//note that ADSC reads HIGH as long as conversion is in progress, goes LOW when conversion is complete
			
			
	uint16_t adc_read = ADCL;
	adc_read = adc_read | (ADCH <<8);
			
	return adc_read;
}