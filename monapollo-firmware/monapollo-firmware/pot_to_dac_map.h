#ifndef POT_TO_DAC_MAP_H
#define POT_TO_DAC_MAP_H

//map pot reads to corresponding DAC MUX and S&H addresses
//define POT to DAC DMUX address map
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

#endif