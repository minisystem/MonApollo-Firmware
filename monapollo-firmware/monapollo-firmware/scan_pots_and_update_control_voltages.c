//This is a big messy routine to read the pots and update the DAC. 
//Really just for testing purposes. Needs to be broken down into pot reading and recording and dac S&H setting

#include <avr/io.h>


#include "dac.h"
#include "adc.h"
#include "hardware.h"
#include "tune.h"
//#include "pot_to_dac_map.h"

//I don't think any of these need to be volatile any more

volatile uint16_t adc_previous = 0;
volatile uint16_t adc_value = 0;
volatile int adc_difference = 0;
int tune_offset = 0; //fine tune offset to display

uint8_t midi_note_number = 0; //store incoming MIDI note here for pitch lookup table

volatile uint16_t value_to_display = 79; //global to hold display value


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
		int fine_offset = 0;
		switch (i)
		{
			case 8: //exception for VCO2 fine
				
				fine_offset = 512 - adc_value;
				set_control_voltage(&fine_cv, vco2_init_cv + tune_offset + fine_offset);
				
				break;
			
			case 9: //exception for TUNE - apply to both VCO1 and VCO2
				adc_difference = adc_value - adc_previous;
				adc_previous = adc_previous + (adc_difference>>2);
				value_to_display = adc_previous;
				tune_offset = 512 - adc_previous;
				set_control_voltage(&tune_cv, vco1_init_cv + tune_offset);
				break;
			
			case 11: //handle ARP_RATE pot
			
				break;
				
			case 4: //LFO pitch modulation depth control - reduce by 1/4
				
				set_control_voltage(&pitch_lfo_cv, adc_value << 2);	//1/4 scale allows for easier vibrato settings
				break;
			
			default: //set control voltage full-scale
				set_control_voltage(pot_decoder_0[i], adc_value << 4);
				break;
			
		}
		//if (i == 8 || i == 9) //exception to handle tune and fine for VCO1 and VCO2
		//{
			//uint16_t tune_value = vco2_init_cv;//6303;//9759; //init CV offset of about -5.8V
			//if (i == 9) tune_value = vco1_init_cv;//-= 1638; //add an octave (1V) to VCO2 pitch
			//if (adc_value >= 512) {
				//set_control_voltage(pot_decoder_0[i],(tune_value + (adc_value - 512)));
				//tune_offset = adc_value - 512;
			//} else {
				//set_control_voltage(pot_decoder_0[i],(tune_value - (512- adc_value)));
				//tune_offset = adc_value;
			//}
//
		//} else if (i == 11) //exception to handle ARP_RATE pot
		//{
			////store ARP pot value, but don't set DAC
			//
		//} else {
			//set_control_voltage(pot_decoder_0[i], adc_value << 4);
		//}
		
	}
	
	//now read second set of pots form U4 and set appropriate DAC S&H channel
	for (int i = 0; i <=14; i++) //first U4 input is grounded - only 15 pots, not 16 on second mux
	{
		
		adc_value = read_pot(POTMUX_EN1, i+1);
		set_control_voltage(pot_decoder_1[i], adc_value <<4);

	}
	
	//set VCO1 and VCO2 pitch control voltages. Remember, set_control_voltage() is expecting a pointer to a control_voltage struct
	//that contains the control_voltage multiplexer channel and the multiplexer address
	set_control_voltage(&vco1_pitch_cv, vco1_pitch_table[midi_note_number]);
	
	set_control_voltage(&vco2_pitch_cv, vco2_pitch_table[midi_note_number + 12]);
	
	DAC_CTRL &= ~(1<<DAC_RS); //reset DAC
	DAC_CTRL |= (1<<DAC_RS);	
	
	
}