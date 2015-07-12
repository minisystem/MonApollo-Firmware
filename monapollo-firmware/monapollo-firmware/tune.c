#include <avr/io.h>
#include "tune.h"
#include "dac.h"
#include "hardware.h"

volatile uint8_t period_counter = 0;
volatile uint8_t no_overflow = TRUE;
volatile uint8_t count_finished = FALSE;
volatile uint16_t osc_count = 0;

volatile uint16_t vco1_init_cv = 0;
volatile uint16_t vco2_init_cv = 0;

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
	
	//these variables hold VCO specific parameters
	uint8_t switch_byte = 0;
	struct control_voltage *vco_init_cv;
	struct control_voltage *vco_mix_cv;
	struct control_voltage *vco_pw_cv;
	struct control_voltage *vco_pitch_cv;
	
	
	count_finished = FALSE;
	if (vco == VCO1) { //turn on VCO1 pulse
		//this will change in v1.1 of analog board when comparator is used to generate pulse for T0 pin
		//turn on VCO1 pulse, all others off
		switch_byte |= (1<<VCO1_PULSE);
		vco_init_cv = &tune_cv; //VCO1 init CV currently mapped to tune_cv - need to rename tune_cv to vco1_init_cv
		vco_mix_cv = &vco1_mix_cv;
		vco_pw_cv = &vco1_pw_cv;
		vco_pitch_cv = &vco1_pitch_cv; //need to keep this 0V during initial pitch setting
		
	} else { //turn on VCO2 pulse
		
		//turn on VCO2 pulse, all others off
		switch_byte |= (1<<VCO2_PULSE);
		vco_init_cv = &fine_cv;	//VCO2 initi CV currently mapped to fine_cv - need to rename fine_cv to vco2_init_cv
		vco_mix_cv = &vco2_mix_cv;
		vco_pw_cv = &vco2_pw_cv;
		vco_pitch_cv = &vco2_pitch_cv; //need to keep this 0V during initial pitch setting
		
	}
	
	//latch switch data
	DATA_BUS = switch_byte;
	VCO_SW_LATCH_PORT |= (1<<VCO_SW_LATCH);
	//_delay_us(1); //why is this delay here????
	VCO_SW_LATCH_PORT &= ~(1<<VCO_SW_LATCH);
	DATA_BUS = 0;

	set_control_voltage(&noise_mix_cv, MIN); //turn noise off
	
	PORTF |= (1<<GATE); //turn gate on
	
	//set up timer/counter0 to be clocked by T0 input
	
	TCCR0A |= (1<<CS02) | (1<<CS01) | (1<<CS00) | (1<<WGM01); //clocked by external T0 pin, rising edge + clear timer on compare match
	OCR0A = 1; //output compare register - set to number of periods to be counted. OCR0A needs to be set to (periods_to_be_counted - 1)
	TIMSK0 |= (1<<OCIE0A); //enable output compare match A interrupt	
	
	for (int dac_bit = 13; dac_bit >= 0; dac_bit--) {
	
		init_cv |= (1<<dac_bit);
		
		set_control_voltage(vco_init_cv, init_cv);
		
		count_finished = FALSE;
		period_counter = 0;
		//TIMSK0 |= (1<<OCIE0A); //enable output compare match A interrupt
		
		while (count_finished == FALSE) { //need to have a watchdog timer here to escape while loop if it takes too long
			
				set_control_voltage(vco_init_cv, init_cv);
				set_control_voltage(vco_pw_cv, MAX);
				set_control_voltage(&volume_cv, MIN);
				set_control_voltage(&cutoff_cv, MAX);
				set_control_voltage(&sustain_1_cv, MAX);
				set_control_voltage(&sustain_2_cv, MAX); //can't remember is EG1 for VCA or EG2????
				set_control_voltage(vco_mix_cv, MAX);
				set_control_voltage(vco_pitch_cv, 0);	
			
		}
		//remember that as INIT_CV goes up, pitch goes down, so looking for osc_count >= reference_count instead of <= as is the case for normal oscillator tuning
		//similarily, OR no_overflow == FALSE not AND no_overflow == FALSE to clear bits that make initial pitch too low
		if ((osc_count >= 9552)  || (no_overflow == FALSE)) init_cv &= ~(1 << dac_bit);
		no_overflow = TRUE;
		
	}		
	
	//none of these help with clicking when returning from this function and starting to read pots	
	set_control_voltage(&release_1_cv, MIN); //this will hopefully reduce popping after returning from initializing pitch CV
	set_control_voltage(&release_2_cv, MIN);
	set_control_voltage(&cutoff_cv, MIN);
		
	PORTF &= ~(1<<GATE); //turn gate off
	
	TIMSK0 &= ~(1<<OCIE0A); //turn off compare match A interrupt
	
	return init_cv;
	
}