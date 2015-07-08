#ifndef TUNE_H
#define TUNE_H

//TIMER COUNTS FOR TUNING OVER ONE OCTAVE

#define PITCH_C     7645//30578//38223
#define PITCH_Cb    7215//28862//36077
#define PITCH_D     6810//27242//34052
#define PITCH_Db    6428//25713//32141
#define PITCH_E     6067//24270//30337
#define PITCH_F     5727//22908//28635
#define PITCH_Fb    5405//21622//27027
#define PITCH_G     5102//20408//25511
#define PITCH_Gb    4816//19263//24079
#define PITCH_A     4545//18182//22727
#define PITCH_Ab    4290//17161//21452
#define PITCH_B     4050//16198//20248

#define VCO1		0
#define VCO2		1

uint16_t set_vco_init_cv(uint8_t vco); //returns 14 bit OSC_INIT_CV




#endif