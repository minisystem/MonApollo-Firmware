#include <avr/io.h>
#include "tune.h"
#include "dac.h"
#include "hardware.h"


uint16_t set_vco_init_cv(uint8_t vco) {
	
	uint16_t init_cv;
	
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

	
	if (vco == VCO1) { //turn on VCO1 pulse
		//this will change in v1.1 of analog board when comparator is used to generate pulse for T0 pin
		set_control_voltage(&vco1_pw_cv, 0x3000); //set pulse width to about 75%
		set_control_voltage(&vco1_mix_cv, MAX); //turn up VCO1 in mixer
		set_control_voltage(&vco2_mix_cv, MIN); //turn off VCO2 in mixer
		//turn on VCO1 pulse, all others off
		switch_byte |= (1<<VCO1_PULSE);
		
		
	} else { //turn on VCO2 pulse
		
		set_control_voltage(&vco2_pw_cv, 0x3000); //set pulse width to about 75%
		set_control_voltage(&vco2_mix_cv, MAX); //turn up VCO2 in mixer
		set_control_voltage(&vco1_mix_cv, MIN); //turn off VCO1 in mixer
		//turn on VCO2 pulse, all others off
		switch_byte |= (1<<VCO2_PULSE);
			
		
	}
	
	//latch switch data
	DATA_BUS = switch_byte;
	VCO_SW_LATCH_PORT |= (1<<VCO_SW_LATCH);
	//_delay_us(1); //why is this delay here????
	VCO_SW_LATCH_PORT &= ~(1<<VCO_SW_LATCH);
	DATA_BUS = 0;	
	
	//set up 8 bit and 16 bit timers for tuning
	
	
	
	return init_cv;
	
}