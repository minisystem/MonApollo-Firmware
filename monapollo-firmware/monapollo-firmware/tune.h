#ifndef TUNE_H
#define TUNE_H

//TIMER COUNTS FOR TUNING OVER ONE OCTAVE

#define PITCH_C     38223
#define PITCH_Cb    36077
#define PITCH_D     34052
#define PITCH_Db    32141
#define PITCH_E     30337
#define PITCH_F     28635
#define PITCH_Fb    27027
#define PITCH_G     25511
#define PITCH_Gb    24079
#define PITCH_A     22727
#define PITCH_Ab    21452
#define PITCH_B     20248

#define VCO1		0
#define VCO2		1



extern volatile uint8_t period_counter;
extern volatile uint8_t period;
extern volatile uint8_t timer1_clock;
extern volatile uint8_t no_overflow;
extern volatile uint8_t count_finished;
extern volatile uint16_t osc_count;
extern volatile uint16_t vco1_init_cv;
extern volatile uint16_t vco2_init_cv;

extern volatile uint8_t compare_match_counter;

extern uint16_t vco1_pitch_table[17]; 
extern uint16_t vco2_pitch_table[17];

uint16_t set_vco_init_cv(uint8_t vco, uint16_t base_reference); //returns 14 bit OSC_INIT_CV

void tune_octave(uint8_t octave, uint8_t vco); //fill pitch tables for specified octave

void tune_8ths(uint8_t vco); //tune every 8th MIDI note

#endif