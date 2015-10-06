#define F_CPU 20000000UL

#include <avr/io.h>
#include <util/delay.h>

#include "hardware.h"
#include "dac.h"

struct control_voltage vco1_pitch_cv	={VCO1_PITCH, DAC_MUX_EN1};
struct control_voltage vco2_pitch_cv	={VCO2_PITCH, DAC_MUX_EN1};	

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

void set_control_voltage (struct control_voltage * cv, uint16_t value) {

	DAC_BUS_LOW = value & 0b00000011111111; //mask top 6 MSBs to set low byte
	
	DAC_BUS_HIGH = value >> 8; //shift away bottom LSBs to set high byte
	
	DAC_CTRL &= ~(1<<DAC_WR); //write DATA
	DAC_CTRL |= (1<<DAC_WR);	

	DATA_BUS = cv->channel; //set channel for DG408 multiplexer output

	_delay_us(1); //AD5556 DAC has 0.5 us settling time. 1 us wasn't long enough for transitions from 10V to 0V
	DAC_MUX |= (1<<cv->mux_addr); //enable multiplexer
	_delay_us(1); //wait for S&H cap to charge - need to figure out how to do this more time efficiently
	DAC_MUX &= ~(1<<cv->mux_addr); //disable multiplexer
	
}	

	
void setup_dac(void) //set up DAC
{
	DDRG |= (1<<DAC_WR) | (1<<DAC_RS); //set DAC control bits as outputs
	DDRD = 0xFF; //set DAC_BUS_LOW bits to outputs
	DDRC |= 0xFF;//set DAC_BUS_HIGH bits to outputs
	DDRH |= (1<<DAC_MUX_EN0) | (1<<DAC_MUX_EN1) | (1<<DAC_MUX_EN2) | (1<<DAC_MUX_EN3); //set DAC_MUX_EN pins as outputs
	
	DAC_MUX &= ~((1<<DAC_MUX_EN0) | (1<<DAC_MUX_EN1) | (1<<DAC_MUX_EN2) | (1<<DAC_MUX_EN3)); //disable DG408 VDAC multiplexers
	
	DAC_CTRL |= (1<<DAC_RS) | (1<<DAC_WR); //disable DAC
	
	DAC_CTRL &= ~(1<<DAC_RS); //reset DAC
	DAC_CTRL |= (1<<DAC_RS);
	
	DAC_CTRL &= ~(1<<DAC_WR); //write DATA - falling edge then rising edge to toggle DAC bits to output register
	DAC_CTRL |= (1<<DAC_WR);
}



