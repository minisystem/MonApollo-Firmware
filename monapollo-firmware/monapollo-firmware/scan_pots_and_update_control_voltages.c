//need to move this into someother file. Maybe put both in synth.c?

#include <avr/io.h>

#include "dac.h"
#include "adc.h"
#include "hardware.h"
#include "tune.h"
#include "assigner.h"
#include "synth.h"
#include "clock.h"
#include "arp.h"


static uint16_t adc_value = 0;

uint8_t midi_note_number = 0; //store incoming MIDI note here for pitch lookup table

volatile uint16_t value_to_display = 79; //global to hold display value
	
void scan_pots(void) { //should probably move this to adc.c

	int adc_change = 0;
	
	uint16_t *patch_value = &(current_patch.vco2_mix); //pointer to first element of current_patch struct
	//scan 30 parameter pots
	for (int i = 0; i <= NUM_POTS; i++) {
		
		adc_value = read_pot(pot_id[i]);
		adc_change = adc_value - pot_id[i]->value;
		pot_id[i]->value = pot_id[i]->value + (adc_change >> 2);
		//what happens next depends on mode. if pot is locked, then the value of the pot is not written to the current patch unless it is different from the pot's locked value
		
		uint8_t delta_pot = pot_id[i]->locked_value - ((pot_id[i]->value >> 2)); //quick and dirty subtraction, where unsigned delta pot will overflow if value > locked value. see below
		
		
		if ((current_patch.mode == MANUAL) || (pot_id[i]->locked == 0)) { //if in manual mode or pot is already unlocked
			
			*(patch_value + i) = pot_id[i]->value; //this is a hacked way of indexing the patch structure. Depends on order of pots in pot array being the same as order of parameters in patch struct
			
		//} else if ((((pot_id[i]->value >> 2) != pot_id[i]->locked_value)) && (pot_id[i]->locked == 1)) { //need to figure out delta threshold here. 10 bit to 8 bit resolution should be enough???
			
		} else if (((delta_pot > 2) && (delta_pot < 253)) && (pot_id[i]->locked == 1)) { //set a threshold of +/- 3 for pot change  		  		  	
			*(patch_value + i) = pot_id[i]->value;
			pot_id[i]->locked = 0; //unlock pot
			current_patch.mode = EDIT;
			
		}
		
		//otherwise, the pot value is assigned to it's corresponding parameter in the current_patch struct:
		
	}
	
	
	//scan volume pot
	adc_value = read_pot(&volume_pot);
	adc_change = adc_value - volume_pot.value;
	volume_pot.value = volume_pot.value + (adc_change >> 2);
	
}

void update_control_voltages(void) { //keep everything updated in the current order of pots. Probably arbitrary, but try to minimize change from old CV update to new CV update
	
	set_control_voltage(&vco2_mix_cv, (current_patch.vco2_mix << 4));
	set_control_voltage(&vco1_mix_cv, (current_patch.vco1_mix << 4));
	set_control_voltage(&pitch_eg2_cv, (current_patch.pitch_eg2 << 3)); //1/4 scale
	set_control_voltage(&pitch_vco2_cv, (current_patch.pitch_vco2 << 4));
	set_control_voltage(&pitch_lfo_cv, (current_patch.pitch_lfo << 3)); // 1/4 scale
	set_control_voltage(&pwm_lfo_cv, (current_patch.pwm_lfo) << 4);
	set_control_voltage(&pwm_eg2_cv, (current_patch.pwm_eg2) << 4);
	set_control_voltage(&vco1_pw_cv, (current_patch.vco1_pw) << 4);
	
	int tune_offset = 512 - current_patch.tune; //master tune offset
	
	int fine_offset = 512 - current_patch.fine; //fine tune offset
	
	set_control_voltage(&fine_cv, vco2_init_cv + tune_offset + fine_offset);
	set_control_voltage(&tune_cv, vco1_init_cv + tune_offset);
	
	set_control_voltage(&lfo_rate_cv, (current_patch.lfo_rate) << 4);
	set_control_voltage(&glide_cv, (current_patch.glide) << 4);
	set_control_voltage(&amp_lfo_cv, (current_patch.amp_lfo) << 4);
	set_control_voltage(&volume_cv, (volume_pot.value << 4)); //volume level not a patch parameter
	set_control_voltage(&vco2_pw_cv, (current_patch.vco2_pw) << 4);	
	
	set_control_voltage(&fil_eg2_cv, (current_patch.fil_eg2) << 4);
	set_control_voltage(&res_cv, (current_patch.res) << 4);
	
	//this next bit should be separated out, but leave it here for now while testing decoupled adc/dac read/write
	uint8_t note = get_current_note(); //get current note from assigner
	if (note < 8) note = 8; //init_cv gives VCO range from MIDI note 8 to MIDI note 127+. If you don't set notes <8 to 8 then you get array out of bounds problems. Should find a better way to handle this.
	//value_to_display = current_patch.number + 100*(arp.sequence[arp.step_position].note); //100*note;
	//value_to_display = arp.sequence[arp.step_position].note;
	//uint8_t arp_note = arp.sequence[arp.step_position].note;
	//if (arp_note == EMPTY) arp_note = 0;
	value_to_display = current_patch.number + 100*arp.display;
		
	uint16_t interpolated_pitch_cv = 0; //holder for interpolated pitch values
	
	interpolated_pitch_cv = interpolate_pitch_cv(note-8, filter_pitch_table); //subtract 8 from note because filter pitch is calibrated so that 0V is E, 20.6 Hz
	//note that product of key_track and interpolated_pitch_cv needs to be cast as uint32t - otherwise product is evaluated incorrectly
	uint16_t divided_pitch_cv = ((uint32_t)current_patch.key_track*interpolated_pitch_cv) >> 10;
	uint16_t filter_cutoff_cv = divided_pitch_cv + (current_patch.cutoff << 4); //filter cutoff CV is the sum of filter cutoff pot and key track amount.
	if (filter_cutoff_cv > MAX) filter_cutoff_cv = MAX; //make sure there is no overflow/wrap by capping max
	set_control_voltage(&cutoff_cv, filter_cutoff_cv);	
	
	set_control_voltage(&fil_vco2_cv, (current_patch.fil_vco2) << 4);
	set_control_voltage(&fil_lfo_cv, (current_patch.fil_lfo) << 4);
	set_control_voltage(&noise_mix_cv, (current_patch.noise_mix) << 4);
	set_control_voltage(&attack_2_cv, (current_patch.attack_2) << 4);
	set_control_voltage(&attack_1_cv, (current_patch.attack_1) << 4);
	set_control_voltage(&decay_2_cv, (current_patch.decay_2) << 4);
	set_control_voltage(&decay_1_cv, (current_patch.decay_1) << 4);
	set_control_voltage(&sustain_1_cv, (current_patch.sustain_1) << 4);
	set_control_voltage(&sustain_2_cv, (current_patch.sustain_2) << 4);
	set_control_voltage(&release_1_cv, (current_patch.release_1) << 4);
	set_control_voltage(&release_2_cv, (current_patch.release_2) << 4);

	//set VCO1 and VCO2 pitch control voltages. Remember, set_control_voltage() is expecting a pointer to a control_voltage struct
	//that contains the control_voltage multiplexer channel and the multiplexer address
	

	uint8_t vco1_note = transpose_note(note, VCO1); //transpose

	interpolated_pitch_cv = interpolate_pitch_cv(vco1_note, vco1_pitch_table);
	
	//value_to_display = interpolated_pitch_cv;
	
	set_control_voltage(&vco1_pitch_cv, interpolated_pitch_cv);
	
	uint8_t vco2_note = transpose_note(note, VCO2);
	
	interpolated_pitch_cv = interpolate_pitch_cv(vco2_note, vco2_pitch_table);
	
	set_control_voltage(&vco2_pitch_cv, interpolated_pitch_cv);
		
	DAC_CTRL &= ~(1<<DAC_RS); //reset DAC
	DAC_CTRL |= (1<<DAC_RS);
	
	system_clock.rate = (1023 - arp_rate_pot.value) + 244;    
	
	if (system_clock.rate != system_clock.previous_rate) {
		
		update_clock_rate(system_clock.rate);
	
	}
	
	system_clock.previous_rate = system_clock.rate;
	
}			
	
