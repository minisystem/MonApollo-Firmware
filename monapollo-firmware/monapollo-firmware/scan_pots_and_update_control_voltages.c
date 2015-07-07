//This is a big messy routine to read the pots and update the DAC. 
//Really just for testing purposes. Needs to be broken down into pot reading and recording and dac S&H setting

#include <avr/io.h>


#include "dac.h"
#include "adc.h"
#include "hardware.h"
#include "pot_to_dac_map.h"

//I don't think any of these need to be volatile any more

volatile uint16_t adc_previous = 0;
volatile uint16_t adc_value = 0;
volatile uint16_t tune_offset = 0; //fine tune offset to display


struct control_voltage vco1_pitch_cv = {VCO1_PITCH, DAC_MUX_EN1};
struct control_voltage vco2_pitch_cv = {VCO2_PITCH, DAC_MUX_EN1};	

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
	{0,				0}, //ARP_RATE - pot is read by ADC but nothing is written to DAC os use dummy data for now
	{GLIDE,			DAC_MUX_EN1},
	{AMP_LFO,		DAC_MUX_EN2},
	{VOLUME,		DAC_MUX_EN2},
	{VCO2_PW,		DAC_MUX_EN1}															
};



struct control_voltage vco2_mix_cv		={VCO2_MIX,		DAC_MUX_EN1};
struct control_voltage vco1_mix_cv		={VCO1_MIX,		DAC_MUX_EN1};
struct control_voltage pitch_eg2_cv		={PITCH_EG2,	DAC_MUX_EN1};	
struct control_voltage pitch_vco2_cv	={PITCH_VCO2,	DAC_MUX_EN0};	
struct control_voltage pitch_lfo_cv		={PITCH_LFO,	DAC_MUX_EN0};
struct control_voltage pwm_lfo_cv		={PWM_LFO,		DAC_MUX_EN0};
struct control_voltage pwm_eg2_cv		={PWM_EG2,		DAC_MUX_EN0};	
struct control_voltage vco1_pw_cv		={VCO1_PW,		DAC_MUX_EN0};
struct control_voltage fine_cv			={FINE,			DAC_MUX_EN1};
struct control_voltage tune_cv			={TUNE,			DAC_MUX_EN0};
struct control_voltage lfo_rate_cv		={LFO_RATE,		DAC_MUX_EN0};
struct control_voltage glide_cv			={GLIDE,		DAC_MUX_EN1};		
struct control_voltage amp_lfo_cv		={AMP_LFO,		DAC_MUX_EN2};
struct control_voltage volume_cv		={VOLUME,		DAC_MUX_EN2};
struct control_voltage vco2_pw_cv		={VCO2_PW,		DAC_MUX_EN1};

struct control_voltage arp_rate_null	={0,0}; //null control voltage for arp rate pointer (only pot that does not does not have its value used to set a control voltage by the DAC)

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

struct control_voltage fil_eg2_cv		={FIL_EG2,		DAC_MUX_EN2};
struct control_voltage res_cv			={RES,			DAC_MUX_EN2};
struct control_voltage cutoff_cv		={CUTOFF,		DAC_MUX_EN2};
struct control_voltage key_track_cv		={KEY_TRACK,	DAC_MUX_EN2};
struct control_voltage fil_vco2_cv		={FIL_VCO2,		DAC_MUX_EN2};
struct control_voltage fil_lfo_cv		={FIL_LFO,		DAC_MUX_EN2};
struct control_voltage noise_mix_cv		={NOISE_MIX,	DAC_MUX_EN0};
struct control_voltage attack_2_cv		={ATTACK_2,		DAC_MUX_EN3};
struct control_voltage attack_1_cv		={ATTACK_1,		DAC_MUX_EN3};
struct control_voltage decay_2_cv		={DECAY_2,		DAC_MUX_EN3};
struct control_voltage decay_1_cv		={DECAY_1,		DAC_MUX_EN3};
struct control_voltage sustain_2_cv		={SUSTAIN_2,	DAC_MUX_EN3};
struct control_voltage sustain_1_cv		={SUSTAIN_1,	DAC_MUX_EN3};
struct control_voltage release_2_cv		={RELEASE_2,	DAC_MUX_EN3};
struct control_voltage release_1_cv		={RELEASE_1,	DAC_MUX_EN3};						 							 			
	
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
		
		if (i == 8 || i == 9) //exception to handle tune and fine for VCO1 and VCO2
		{
			uint16_t tune_value = 6303;//9759; //init CV offset of about -5.8V
			if (i == 9) tune_value += 1638; //add an octave (1V) to VCO2 pitch
			if (adc_value >= 512) {
				//set_dac(dac_pot_decoder_0[i][1],dac_pot_decoder_0[i][0],(tune_value + (adc_value - 512)));
				set_control_voltage(pot_decoder_0[i],(tune_value + (adc_value - 512)));
				tune_offset = adc_value - 512;
			} else {
				//set_dac(dac_pot_decoder_0[i][1],dac_pot_decoder_0[i][0],(tune_value - (512- adc_value)));
				set_control_voltage(pot_decoder_0[i],(tune_value - (512- adc_value)));
				tune_offset = adc_value;
			}

		} else if (i == 11) //exception to handle ARP_RATE pot
		{
			//store ARP pot value, but don't set DAC
			
		} else {
			//set_dac(dac_pot_decoder_0[i][1],dac_pot_decoder_0[i][0], adc_value << 4);
			set_control_voltage(pot_decoder_0[i], adc_value << 4);
		}
		
	}
	
	//now read second set of pots form U4 and set appropriate DAC S&H channel
	for (int i = 0; i <=14; i++) //first U4 input is grounded - only 15 pots, not 16 on second mux
	{
		
		//DATA_BUS = i+1; //U4 input 0 is not used (grounded)
		//POT_MUX &= ~(1<<POTMUX_EN1);
		//_delay_us(2); //ADC settling time. Previously used 10 us, testing 2 us now.
		//ADCSRA |= (1<<ADSC); //start ADC conversion
		//while ((ADCSRA & (1<<ADSC))); //wait for ADC conversion to complete (13 cycles of ADC clock) - need to figure out what to do with this time - would interrupt be more efficient?
		//POT_MUX |= (1<<POTMUX_EN1); //disable pot multiplexer U2
		////adc_previous = adc_value;
		//adc_value = ADCL;
		//adc_value = adc_value | (ADCH <<8);
		
		adc_value = read_pot(POTMUX_EN1, i+1);
		
		//set_dac(dac_pot_decoder_1[i][1],dac_pot_decoder_1[i][0], adc_value << 4);
		set_control_voltage(pot_decoder_1[i], adc_value <<4);

	}
	
	//set_dac(DAC_MUX_EN1, VCO1_PITCH, 0); //OSCA PITCH CV
	//set_dac(DAC_MUX_EN1, VCO2_PITCH, 0); //OSCB PITCH CV

	set_control_voltage(&vco1_pitch_cv, 0);
	set_control_voltage(&vco2_pitch_cv, 0);
	
	DAC_CTRL &= ~(1<<DAC_RS); //reset DAC
	DAC_CTRL |= (1<<DAC_RS);	
	
	
}