#include <avr/io.h>
#include "tune.h"
#include "dac.h"
#include "hardware.h"

volatile uint8_t period_counter = 0;
volatile uint8_t no_overflow = TRUE;
volatile uint8_t count_finished = FALSE;
volatile uint16_t osc_count = 0;

uint16_t set_vco_init_cv(uint8_t vco) {
	
	uint16_t init_cv = 0;
	
	//disable main scanning interrupt
	
	//disable UART so MIDI data doesn't interrupt tuning
	
	//setup control voltages	
	set_control_voltage(&volume_cv, MIN); //turn volume all the way down
	//turn off all pitch modulation
	set_control_voltage(&pitch_lfo_cv, MIN);
	set_control_voltage(&pitch_eg2_cv, MIN);
	set_control_voltage(&pitch_vco2_cv, MIN);
	//turn off glide
	set_control_voltage(&glide_cv, MIN);
	//turn off all pulse width modulation
	set_control_voltage(&pwm_eg2_cv, MIN);
	set_control_voltage(&pwm_lfo_cv, MIN);
	//turn off all filter modulation
	set_control_voltage(&fil_lfo_cv, MIN);
	set_control_voltage(&fil_eg2_cv, MIN);
	set_control_voltage(&fil_vco2_cv, MIN);
	set_control_voltage(&key_track_cv, MIN);
	//open filter with no resonance
	set_control_voltage(&cutoff_cv, MAX);
	set_control_voltage(&res_cv, MIN);
	//turn off VCA LFO modulation
	set_control_voltage(&amp_lfo_cv, MIN);
	//initialize VCA envelope
	set_control_voltage(&attack_1_cv, MIN);
	set_control_voltage(&decay_1_cv, MIN);
	set_control_voltage(&sustain_1_cv, MAX);
	set_control_voltage(&release_1_cv, MIN);
	//turn off noise
	set_control_voltage(&noise_mix_cv, MIN);
	
	
	uint8_t switch_byte = 0;
	struct control_voltage *vco_init_cv;
	
	if (vco == VCO1) { //turn on VCO1 pulse
		//this will change in v1.1 of analog board when comparator is used to generate pulse for T0 pin
		set_control_voltage(&vco1_pw_cv, 0x3000); //set pulse width to about 75%
		set_control_voltage(&vco1_mix_cv, MAX); //turn up VCO1 in mixer
		set_control_voltage(&vco2_mix_cv, MIN); //turn off VCO2 in mixer
		//turn on VCO1 pulse, all others off
		switch_byte |= (1<<VCO1_PULSE);
		vco_init_cv = &tune_cv; //VCO1 init CV currently mapped to tune_cv - need to rename tune_cv to vco1_init_cv
		
	} else { //turn on VCO2 pulse
		
		set_control_voltage(&vco2_pw_cv, 0x3000); //set pulse width to about 75%
		set_control_voltage(&vco2_mix_cv, MAX); //turn up VCO2 in mixer
		set_control_voltage(&vco1_mix_cv, MIN); //turn off VCO1 in mixer
		//turn on VCO2 pulse, all others off
		switch_byte |= (1<<VCO2_PULSE);
		vco_init_cv = &fine_cv;	//VCO2 initi CV currently mapped to fine_cv - need to rename fine_cv to vco2_init_cv
		
	}
	
	//latch switch data
	DATA_BUS = switch_byte;
	VCO_SW_LATCH_PORT |= (1<<VCO_SW_LATCH);
	//_delay_us(1); //why is this delay here????
	VCO_SW_LATCH_PORT &= ~(1<<VCO_SW_LATCH);
	DATA_BUS = 0;

	//set up timer/counter0 to be clocked by T0 input
	TCCR0A |= (1<<CS02) | (1<<CS01) | (1<<CS00); //clocked by external T0 pin, rising edge
	OCR0A = 1; //output compare register - set to number of periods to be counted.
	
	PORTF |= (1<<GATE); //turn gate on
	
	
	
	//for (int dac_bit = 13; dac_bit >= 0; dac_bit--) {
	//
		//init_cv |= dac_bit;
		//
		//set_control_voltage(&tune_cv, init_cv);
		//
		//count_finished = FALSE;
		//period_counter = 0;
		//TIMSK0 |= (1<<OCIE0A); //enable output compare match A interrupt
		//
		//while (count_finished == FALSE) {
			//
			//set_control_voltage(&tune_cv, init_cv);
			//
		//}
		//
		//if ((osc_count <= 38226)  && (no_overflow == TRUE)) init_cv &= ~(1 << dac_bit);
		//no_overflow = TRUE;
		//
	//}		
	
	//set_control_voltage(&tune_cv, 9500);	
	TIMSK0 |= (1<<OCIE0A);	
	
	while (period_counter == 0) {}
		
	PORTF &= ~(1<<GATE); //turn gate off
	
	TIMSK0 &= ~(1<<OCIE0A); //turn off compare match A interrupt
	
	return init_cv;
	
}