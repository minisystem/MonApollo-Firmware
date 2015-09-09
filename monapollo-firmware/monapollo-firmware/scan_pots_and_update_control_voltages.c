//This is a big messy routine to read the pots and update the DAC. 
//Really just for testing purposes. Needs to be broken down into pot reading and recording and dac S&H setting

#include <avr/io.h>

#include "dac.h"
#include "adc.h"
#include "hardware.h"
#include "tune.h"
#include "assigner.h"
//#include "pot_to_dac_map.h"

//I don't think any of these need to be volatile any more

volatile uint16_t adc_previous = 0;
volatile uint16_t adc_value = 0;
volatile int adc_difference = 0;
int tune_offset = 0; //fine tune offset to display

uint8_t midi_note_number = 0; //store incoming MIDI note here for pitch lookup table

volatile uint16_t value_to_display = 79; //global to hold display value

//arrays to hold ADC values for each pot - needed for digital filtering of adc reads, plus will be able to be used for parameter storage and recall
uint16_t pot_group_0[16] = {0}; 
uint16_t pot_group_1[15] = {0};	

//First group of pots inputs 0-15 on U2 demulitplexer
//This is an array of pointers to control_voltage structs
struct control_voltage *pot_decoder_0[16] = {
	
	&vco2_mix_cv,
	&vco1_mix_cv,
	&pitch_eg2_cv,
	&pitch_vco2_cv,
	&pitch_lfo_cv,
	&pwm_lfo_cv,
	&pwm_eg2_cv,
	&vco1_pw_cv,
	&fine_cv,
	&tune_cv,
	&lfo_rate_cv,
	&arp_rate_null,
	&glide_cv,
	&amp_lfo_cv,
	&volume_cv,
	&vco2_pw_cv
		
};
					 							 			
//Second group of pot inputs 1-15 (input 0 is grounded) on U4 demultiplexer
//This is an array of pointers to control_voltage structs	
struct control_voltage *pot_decoder_1[15] = {
	
	&fil_eg2_cv,
	&res_cv,
	&cutoff_cv,
	&key_track_cv,
	&fil_vco2_cv,
	&fil_lfo_cv,
	&noise_mix_cv,
	&attack_2_cv,
	&attack_1_cv,
	&decay_2_cv,
	&decay_1_cv,
	&sustain_2_cv,
	&sustain_1_cv,
	&release_2_cv,
	&release_1_cv
	}; 
	
void scan_pots_and_update_control_voltages(void) {

	//read pots on U2 pot multiplexer and set appropriate DAC S&H channel
	for (int i = 0; i <=15; i++)
	{

		adc_value = read_pot(POTMUX_EN0, i);
		//implement IIR digital low pass filter - change pot value only by some fraction (>>2) of the difference between the new and old value
		int adc_change = adc_value - pot_group_0[i];
		pot_group_0[i] = pot_group_0[i] + (adc_change >> 2);
		int fine_offset = 0;
		
		switch (i)
		{
			case 8: //exception for VCO2 fine
				
				//value_to_display = pot_group_0[i];
				fine_offset = 512 - pot_group_0[i];
				set_control_voltage(&fine_cv, vco2_init_cv + tune_offset + fine_offset);
				
				break;
			
			case 9: //exception for TUNE - apply to both VCO1 and VCO2

				tune_offset = 512 - pot_group_0[i];
				set_control_voltage(&tune_cv, vco1_init_cv + tune_offset);
				break;
			
			case 11: //handle ARP_RATE pot
			
				break;
				
			case 4: //LFO pitch modulation depth control - reduce by 1/4
				
				set_control_voltage(&pitch_lfo_cv, pot_group_0[i] << 3);	//1/4 scale allows for easier vibrato settings
				break;
			
			default: //set control voltage full-scale
				set_control_voltage(pot_decoder_0[i], pot_group_0[i] << 4);
				break;
			
		}
		
	}
	
	//now read second set of pots from U4 and set appropriate DAC S&H channel
	for (int i = 0; i <=14; i++) //first U4 input is grounded - only 15 pots, not 16 on second mux
	{
		
		adc_value = read_pot(POTMUX_EN1, i+1);
		int adc_change = adc_value - pot_group_1[i];
		pot_group_1[i] = pot_group_1[i] + (adc_change >> 2);		
		
		set_control_voltage(pot_decoder_1[i], pot_group_1[i] <<4);

	}
	
	//set VCO1 and VCO2 pitch control voltages. Remember, set_control_voltage() is expecting a pointer to a control_voltage struct
	//that contains the control_voltage multiplexer channel and the multiplexer address
	
	uint8_t note = get_current_note();
	if (note < 8) note = 8; //init_cv gives VCO range from MIDI note 8 to MIDI note 127+. If you don't set notes <8 to 8 then you get array out of bounds problems. Should find a better way to handle this.
	//value_to_display = note;
	uint8_t vco1_note = add_octave_to_note(note, VCO1);
	uint8_t pitch_index = vco1_note>>3;
	uint8_t delta_note = vco1_note - pitch_index*8; //will range from 0 to 7
	
	value_to_display = vco1_note;
		
	uint16_t y0 = vco1_pitch_table[pitch_index -1];
	uint16_t y1 = vco1_pitch_table[pitch_index];
	
	uint16_t interpolated_pitch_cv = y0 + (((y1 - y0)*delta_note)>>3); //mind order of operations here: + is evaluated before >>
	
	set_control_voltage(&vco1_pitch_cv, interpolated_pitch_cv);
	
	uint8_t vco2_note = note;//add_octave_to_note(note, VCO1);
	pitch_index = vco2_note>>3;
	delta_note = vco2_note - pitch_index*8; //will range from 0 to 7
	
	
	y0 = vco2_pitch_table[pitch_index - 1];
	y1 = vco2_pitch_table[pitch_index];
	
	interpolated_pitch_cv = y0 + (((y1 - y0)*delta_note)>>3);
	
	set_control_voltage(&vco2_pitch_cv, interpolated_pitch_cv); 
	
	DAC_CTRL &= ~(1<<DAC_RS); //reset DAC
	DAC_CTRL |= (1<<DAC_RS);	
	
	
}