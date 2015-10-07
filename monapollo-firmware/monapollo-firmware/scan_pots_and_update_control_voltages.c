//This is a big messy routine to read the pots and update the DAC. 
//Really just for testing purposes. Needs to be broken down into pot reading and recording and dac S&H setting

#include <avr/io.h>

#include "dac.h"
#include "adc.h"
#include "hardware.h"
#include "tune.h"
#include "assigner.h"
#include "synth.h"




static uint16_t adc_value = 0;



uint8_t midi_note_number = 0; //store incoming MIDI note here for pitch lookup table

volatile uint16_t value_to_display = 79; //global to hold display value

//arrays to hold ADC values for each pot - needed for digital filtering of adc reads, plus will be able to be used for parameter storage and recall
uint16_t pot_group_0[16] = {0}; 
uint16_t pot_group_1[15] = {0};	
	
	

//First group of pots inputs 0-15 on U2 demulitplexer
//This is an array of pointers to control_voltage structs
//struct control_voltage *pot_decoder_0[16] = {
	//
	//&vco2_mix_cv,
	//&vco1_mix_cv,
	//&pitch_eg2_cv,
	//&pitch_vco2_cv,
	//&pitch_lfo_cv,
	//&pwm_lfo_cv,
	//&pwm_eg2_cv,
	//&vco1_pw_cv,
	//&fine_cv,
	//&tune_cv,
	//&lfo_rate_cv,
	//&arp_rate_null,
	//&glide_cv,
	//&amp_lfo_cv,
	//&volume_cv,
	//&vco2_pw_cv
		//
//};
					 							 			
//Second group of pot inputs 1-15 (input 0 is grounded) on U4 demultiplexer
//This is an array of pointers to control_voltage structs	
//struct control_voltage *pot_decoder_1[15] = {
	//
	//&fil_eg2_cv,
	//&res_cv,
	//&cutoff_cv,
	//&key_track_cv,
	//&fil_vco2_cv,
	//&fil_lfo_cv,
	//&noise_mix_cv,
	//&attack_2_cv,
	//&attack_1_cv,
	//&decay_2_cv,
	//&decay_1_cv,
	//&sustain_2_cv,
	//&sustain_1_cv,
	//&release_2_cv,
	//&release_1_cv
	//}; 
	
	
void scan_pots(void) { //should probably move this to adc.c

	int adc_change = 0;
	
	uint16_t *patch_value = &(current_patch.vco2_mix); //pointer to first element of current_patch struct
	//scan 30 parameter pots
	for (int i = 0; i <= 29; i++) {
		
		adc_value = read_pot(pot_id[i]);
		adc_change = adc_value - pot_id[i]->value;
		pot_id[i]->value = pot_id[i]->value + (adc_change >> 2);
		//what happens next depends on mode. if pot is locked, then the value of the pot is not written to the current patch unless it is different from the pot's locked value
		//otherwise, the pot value is assigned to it's corresponding parameter in the current_patch struct:
		*(patch_value + i) = pot_id[i]->value; //this is a hacked way of indexing the patch structure. Depends on order of pots in pot array being the same as order of parameters in patch struct
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
	//value_to_display = note;
		
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
}			
	
void scan_pots_and_update_control_voltages(void) {
	//
	//int tune_offset = 0;
	////read pots on U2 pot multiplexer and set appropriate DAC S&H channel
	//for (int i = 0; i <=15; i++)
	//{
//
		//adc_value = read_pot(pot_id[i]);
		////implement IIR digital low pass filter - change pot value only by some fraction (>>2) of the difference between the new and old value
		//int adc_change = adc_value - pot_group_0[i];
		//pot_group_0[i] = pot_group_0[i] + (adc_change >> 2);
		//int fine_offset = 0;
		//
		//switch (i)
		//{
			//case 8: //exception for VCO2 fine
				//
				////value_to_display = pot_group_0[i];
				//fine_offset = 512 - pot_group_0[i];
				//set_control_voltage(&fine_cv, vco2_init_cv + tune_offset + fine_offset);			
				//break;
			//
			//case 9: //exception for TUNE - apply to both VCO1 and VCO2
//
				//tune_offset = 512 - pot_group_0[i];
				//set_control_voltage(&tune_cv, vco1_init_cv + tune_offset);
				//break;
			//
			//case 11: //handle ARP_RATE pot
				////value_to_display = pot_group_0[11];
				//break;
				//
			//case 4: //LFO pitch modulation depth control - reduce by 1/4
				//
				//set_control_voltage(&pitch_lfo_cv, pot_group_0[i] << 3);	//1/4 scale allows for easier vibrato settings
				//break;
			//
			//default: //set control voltage full-scale
				//set_control_voltage(pot_decoder_0[i], pot_group_0[i] << 4);
				//break;
			//
		//}
		//
	//}
	//
	//uint8_t note = get_current_note(); //get current note from assigner
	//if (note < 8) note = 8; //init_cv gives VCO range from MIDI note 8 to MIDI note 127+. If you don't set notes <8 to 8 then you get array out of bounds problems. Should find a better way to handle this.
	//value_to_display = note + 8965;	
	//
	//uint16_t interpolated_pitch_cv = 0; //holder for interpolated pitch values
	//
	////now read second set of pots from U4 and set appropriate DAC S&H channel
	//for (int i = 0; i <=14; i++) //first U4 input is grounded - only 15 pots, not 16 on second mux
	//{
		//
		//adc_value = read_pot(pot_id[i+16]);
		//int adc_change = adc_value - pot_group_1[i];
		//pot_group_1[i] = pot_group_1[i] + (adc_change >> 2);
		//
		//switch(i) 
		//{
			//case 2: //exception to handle filter key tracking: use key_track pot setting to determine how much pitch cv contributes to filter cutoff
				//interpolated_pitch_cv = interpolate_pitch_cv(note-8, filter_pitch_table); //subtract 8 from note because filter pitch is calibrated so that 0V is E, 20.6 Hz
				//uint16_t key_track_byte = (pot_group_1[3]); //
				////if (key_track_byte > 1020) key_track_byte = 1024;			 
				//uint16_t divided_pitch_cv = ((uint32_t)key_track_byte*interpolated_pitch_cv) >> 10; //note that product of key_track_byte and interpolated_pitch_cv needs to be cast as uint32t - otherwise product is evaluated incorrectly
//
				////value_to_display = divided_pitch_cv;
							//
				//uint16_t filter_cutoff_cv = divided_pitch_cv + (pot_group_1[i] << 4); //filter cutoff CV is the sum of filter cutoff pot and key track amount.
				//if (filter_cutoff_cv > MAX) filter_cutoff_cv = MAX; //make sure there is no overflow/wrap by capping max
				//set_control_voltage(&cutoff_cv, filter_cutoff_cv);
				////value_to_display = filter_cutoff_cv;
				//break;
			//
			//default:
				//set_control_voltage(pot_decoder_1[i], pot_group_1[i] <<4);
				//break;
		//}		
		//
		//
//
	//}
	
	
//	scan_pots();
//	update_control_voltages();
	
	////set VCO1 and VCO2 pitch control voltages. Remember, set_control_voltage() is expecting a pointer to a control_voltage struct
	////that contains the control_voltage multiplexer channel and the multiplexer address
	//
//
	//uint8_t vco1_note = transpose_note(note, VCO1); //transpose 
//
	//interpolated_pitch_cv = interpolate_pitch_cv(vco1_note, vco1_pitch_table);
	//
	////value_to_display = interpolated_pitch_cv;
	//
	//set_control_voltage(&vco1_pitch_cv, interpolated_pitch_cv);
	//
	//uint8_t vco2_note = transpose_note(note, VCO2);
	//
	//interpolated_pitch_cv = interpolate_pitch_cv(vco2_note, vco2_pitch_table);
	//
	//set_control_voltage(&vco2_pitch_cv, interpolated_pitch_cv); 
	//
//	DAC_CTRL &= ~(1<<DAC_RS); //reset DAC
//	DAC_CTRL |= (1<<DAC_RS);	
	
	
}