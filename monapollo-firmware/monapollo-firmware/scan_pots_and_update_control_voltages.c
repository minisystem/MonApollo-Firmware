#include <avr/io.h>
#include <util/delay.h>

#include "dac.h"
#include "adc.h"
#include "port_map.h"
#include "pot_to_dac_map.h"



volatile uint16_t adc_previous = 0;
volatile uint16_t adc_value = 0;
volatile uint16_t tune_offset = 0; //fine tune offset to display



//First group of pots inputs 0-15 on U2 demulitplexer
volatile uint8_t dac_pot_decoder_0 [16][2] = { //[DAC MUX CHANNEL][DAC MUX ADDRESS]

	{VCO2_MIX,		DAC_MUX_EN1}, 
	{VCO1_MIX,		DAC_MUX_EN1},
	{PITCH_EG2,		DAC_MUX_EN1},
	{PITCH_VCO2,	DAC_MUX_EN0},
	{PITCH_LFO,		DAC_MUX_EN0},
	{PWM_LFO,		DAC_MUX_EN0},
	{PWM_EG2,		DAC_MUX_EN0},
	{VCO1_PW,		DAC_MUX_EN0},
	{FINE,			DAC_MUX_EN1},
	{TUNE,			DAC_MUX_EN0},
	{LFO_RATE,		DAC_MUX_EN0},
	{0,				0}, //ARP_RATE - pot is read by ADC but nothing is written to DAC os use dummy data for now -set VOLUME S&H CV as I'm not using it yet
	{GLIDE,			DAC_MUX_EN1},
	{AMP_LFO,		DAC_MUX_EN2},
	{VOLUME,		DAC_MUX_EN2},
	{VCO2_PW,		DAC_MUX_EN1}															
};

//Second group of pot inputs 1-15 (input 0 is grounded) on U4 demultiplexer
volatile uint8_t dac_pot_decoder_1 [15][2] = {
	
	{FIL_EG2,		DAC_MUX_EN2},
	{RES,			DAC_MUX_EN2},
	{CUTOFF,		DAC_MUX_EN2},
	{KEY_TRACK,		DAC_MUX_EN2},
	{FIL_VCO2,		DAC_MUX_EN2},
	{FIL_LFO,		DAC_MUX_EN2},
	{NOISE_MIX,		DAC_MUX_EN0},				
	{ATTACK_2,		DAC_MUX_EN3},
	{ATTACK_1,		DAC_MUX_EN3},
	{DECAY_2,		DAC_MUX_EN3},
	{DECAY_1,		DAC_MUX_EN3},
	{SUSTAIN_2,		DAC_MUX_EN3},
	{SUSTAIN_1,		DAC_MUX_EN3},
	{RELEASE_2,		DAC_MUX_EN3},
	{RELEASE_1,		DAC_MUX_EN3}							
};

void scan_pots_and_update_control_voltages(void) {

	//read pots on U2 pot multiplexer and set appropriate DAC S&H channel
	for (int i = 0; i <=15; i++)
	{
		DATA_BUS = i;
		POT_MUX &= ~(1<<POTMUX_EN0);
		_delay_us(2); //ADC settling time. Previously used 10 us, testing 2 us now.
		ADCSRA |= (1<<ADSC); //start ADC conversion
		while ((ADCSRA & (1<<ADSC))); //wait for ADC conversion to complete (13 cycles of ADC clock) - need to figure out what to do with this time - would interrupt be more efficient?
		POT_MUX |= (1<<POTMUX_EN0); //disable pot multiplexer U2
		//note that ADSC reads HIGH as long as conversion is in progress, goes LOW when conversion is complete
		
		//adc_previous = adc_value;
		adc_value = ADCL;
		adc_value = adc_value | (ADCH <<8);
		
		if (i == 8 || i == 9) //exception to handle tune and fine for VCO1 and VCO2
		{
			uint16_t tune_value = 6303;//9759; //init CV offset of about -5.8V
			if (i == 9) tune_value += 1638; //add an octave (1V) to VCO2 pitch
			if (adc_value >= 512) {
				set_dac(dac_pot_decoder_0[i][1],dac_pot_decoder_0[i][0],(tune_value + (adc_value - 512)));
				tune_offset = adc_value - 512;
			} else {
				set_dac(dac_pot_decoder_0[i][1],dac_pot_decoder_0[i][0],(tune_value - (512- adc_value)));
				tune_offset = adc_value;
			}

		} else if (i == 11) //exception to handle ARP_RATE pot
		{
			//store ARP pot value, but don't set DAC
			
		} else {
			set_dac(dac_pot_decoder_0[i][1],dac_pot_decoder_0[i][0], adc_value << 4);
		}
		
	}
	
	//now read second set of pots form U4 and set approriate DAC S&H channel
	for (int i = 0; i <=14; i++) //first U4 input is grounded - only 15 pots, not 16 on second mux
	{
		
		DATA_BUS = i+1; //U4 input 0 is not used (grounded)
		POT_MUX &= ~(1<<POTMUX_EN1);
		_delay_us(2); //ADC settling time. Previously used 10 us, testing 2 us now.
		ADCSRA |= (1<<ADSC); //start ADC conversion
		while ((ADCSRA & (1<<ADSC))); //wait for ADC conversion to complete (13 cycles of ADC clock) - need to figure out what to do with this time - would interrupt be more efficient?
		POT_MUX |= (1<<POTMUX_EN1); //disable pot multiplexer U2
		//adc_previous = adc_value;
		adc_value = ADCL;
		adc_value = adc_value | (ADCH <<8);

		set_dac(dac_pot_decoder_1[i][1],dac_pot_decoder_1[i][0], adc_value << 4);

	}

	DAC_CTRL &= ~(1<<DAC_RS); //reset DAC
	DAC_CTRL |= (1<<DAC_RS);	
	
	
}