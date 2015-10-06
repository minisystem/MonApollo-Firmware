#ifndef DAC_H
#define DAC_H


//define DAC bits
#define DAC_WR			PG0 //DAC write
#define DAC_RS			PG1 //DAC reset
//define 4 DG408 S&H multiplexer addresses
#define DAC_MUX_EN0		PH0 //DG408 enable is active HIGH
#define DAC_MUX_EN1		PH1
#define DAC_MUX_EN2		PH2
#define DAC_MUX_EN3		PH3

//Define control voltage S&H channel, order and grouping here refers to each of the two pot multiplexers
//because the control voltage is derived from the corresponding pot in manual mode
#define VCO2_MIX		6 //U2 pot demultiplexer bit 0
#define VCO1_MIX		7 //U2 pot demultiplexer bit 1
#define PITCH_EG2		0 //U2 pot demultiplexer bit 2
#define PITCH_VCO2		0 //U2 pot demultiplexer bit 3
#define PITCH_LFO		3 //U2 pot demultiplexer bit 4
#define PWM_LFO			7 //U2 pot demultiplexer bit 5
#define PWM_EG2			2 //U2 pot demultiplexer bit 6
#define VCO1_PW			6 //U2 pot demultiplexer bit 7
#define FINE			5 //U2 pot demultiplexer bit 8
#define TUNE			4 //U2 pot demultiplexer bit 9
#define LFO_RATE		1 //U2 pot demultiplexer bit 10
//bit 11 is ARP_RATE, which is not used to set a DAC
#define GLIDE			3 //U2 pot demultiplexer bit 12
#define AMP_LFO			5 //U2 pot demultiplexer bit 13
#define VOLUME			4 //U2 pot demultiplexer bit 14
#define VCO2_PW			4 //U2 pot demultiplexer bit 15

#define FIL_EG2			2 //U4 pot demultiplexer bit 1 (bit 0 is grounded)
#define RES				7 //U4 pot demultiplexer bit 2
#define CUTOFF			6 //U4 pot demultiplexer bit 3
#define KEY_TRACK		3 //U4 pot demultiplexer bit 4
#define FIL_VCO2		0 //U4 pot demultiplexer bit 5
#define	FIL_LFO			1 //U4 pot demultiplexer bit 6
#define NOISE_MIX		5 //U4 pot demultiplexer bit 7
#define ATTACK_2		1 //U4 pot demultiplexer bit 8
#define ATTACK_1		7 //U4 pot demultiplexer bit 9
#define DECAY_2			0 //U4 pot demultiplexer bit 10
#define DECAY_1			6 //U4 pot demultiplexer bit 11
#define SUSTAIN_2		2 //U4 pot demultiplexer bit 12
#define SUSTAIN_1		4 //U4 pot demultiplexer bit 13
#define RELEASE_2		3 //U4 pot demultiplexer bit 14
#define RELEASE_1		5 //U4 pot demultiplexer bit 15

//define other control voltage DAC mus addresses
#define VCO1_PITCH		2
#define VCO2_PITCH		1

//struct for a control voltage contains mux channel and mux address
struct control_voltage {
	
	uint8_t channel:3;
	uint8_t mux_addr:3;
	
};

//control voltage struct for each CV that needs to be generated by DAC
extern struct control_voltage vco1_pitch_cv;
extern struct control_voltage vco2_pitch_cv;

extern struct control_voltage vco2_mix_cv;
extern struct control_voltage vco1_mix_cv;
extern struct control_voltage pitch_eg2_cv;
extern struct control_voltage pitch_vco2_cv;
extern struct control_voltage pitch_lfo_cv;
extern struct control_voltage pwm_lfo_cv;
extern struct control_voltage pwm_eg2_cv;
extern struct control_voltage vco1_pw_cv;
extern struct control_voltage fine_cv;
extern struct control_voltage tune_cv;
extern struct control_voltage lfo_rate_cv;
extern struct control_voltage glide_cv;
extern struct control_voltage amp_lfo_cv;
extern struct control_voltage volume_cv;
extern struct control_voltage vco2_pw_cv;

extern struct control_voltage arp_rate_null; //arp pot is only pot that does not have control over a CV

extern struct control_voltage fil_eg2_cv;
extern struct control_voltage res_cv;
extern struct control_voltage cutoff_cv;
extern struct control_voltage key_track_cv;
extern struct control_voltage fil_vco2_cv;
extern struct control_voltage fil_lfo_cv;
extern struct control_voltage noise_mix_cv;
extern struct control_voltage attack_2_cv;
extern struct control_voltage attack_1_cv;
extern struct control_voltage decay_2_cv;
extern struct control_voltage decay_1_cv;
extern struct control_voltage sustain_2_cv;
extern struct control_voltage sustain_1_cv;
extern struct control_voltage release_2_cv;
extern struct control_voltage release_1_cv;


void setup_dac(void);



void set_control_voltage (struct control_voltage * cv, uint16_t value);

#endif