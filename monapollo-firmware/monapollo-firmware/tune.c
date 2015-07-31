#include <avr/io.h>
#include <stdio.h>
#include "tune.h"
#include "dac.h"
#include "hardware.h"
#include "display.h"

volatile uint8_t period_counter = 0; //need this to track first interrupt after count started
volatile uint8_t period = 0; //this is the actual period number that OCR0A is set to
volatile uint8_t timer1_clock = 0;
volatile uint8_t no_overflow = TRUE;
volatile uint8_t count_finished = FALSE;
volatile uint16_t osc_count = 0;


volatile uint16_t vco1_init_cv = 0;
volatile uint16_t vco2_init_cv = 0;

uint16_t vco1_pitch_table[128] = {0};
uint16_t vco2_pitch_table[128] = {0};
	
uint16_t set_vco_init_cv(uint8_t vco) {

	uint16_t init_cv = 0;
	 timer1_clock |= (1<<CS11) | (1<<CS10);
	//disable UART so MIDI data doesn't interrupt tuning
	
	//setup control voltages	
	set_control_voltage(&volume_cv, MIN); //turn volume all the way down
	//turn off all pitch modulation
	set_control_voltage(&pitch_lfo_cv, MIN);
	set_control_voltage(&pitch_eg2_cv, MIN);
	set_control_voltage(&pitch_vco2_cv, MIN);
	//turn off glide
	set_control_voltage(&glide_cv, MIN);
	//turn off all pulse width modulation
	set_control_voltage(&pwm_eg2_cv, MIN);
	set_control_voltage(&pwm_lfo_cv, MIN);
	//turn off all filter modulation
	set_control_voltage(&fil_lfo_cv, MIN);
	set_control_voltage(&fil_eg2_cv, MIN);
	set_control_voltage(&fil_vco2_cv, MIN);
	set_control_voltage(&key_track_cv, MIN);
	//open filter with no resonance
	set_control_voltage(&cutoff_cv, MAX);
	set_control_voltage(&res_cv, MIN);
	//turn off VCA LFO modulation
	set_control_voltage(&amp_lfo_cv, MIN);
	//initialize VCA envelope
	set_control_voltage(&attack_1_cv, MIN);
	set_control_voltage(&decay_1_cv, MIN);
	set_control_voltage(&sustain_1_cv, MAX);
	set_control_voltage(&release_1_cv, MIN);
	//turn off noise
	set_control_voltage(&noise_mix_cv, MIN);
	
	//these variables hold VCO specific parameters
	uint8_t switch_byte = 0;
	uint16_t reference_count = 0;
	struct control_voltage *vco_init_cv;
	struct control_voltage *vco_mix_cv;
	struct control_voltage *vco_pw_cv;
	struct control_voltage *vco_pitch_cv;
	
	
	
	count_finished = FALSE;
	if (vco == VCO1) { //turn on VCO1 pulse
		//this will change in v1.1 of analog board when comparator is used to generate pulse for T0 pin
		//turn on VCO1 pulse, all others off
		switch_byte |= (1<<VCO1_SAW);
		vco_init_cv = &tune_cv; //VCO1 init CV currently mapped to tune_cv - need to rename tune_cv to vco1_init_cv
		vco_mix_cv = &vco1_mix_cv;
		vco_pw_cv = &vco1_pw_cv;
		vco_pitch_cv = &vco1_pitch_cv; //need to keep this 0V during initial pitch setting
		reference_count = 19111;//38222; //MIDI note C0
		
	} else { //turn on VCO2 pulse
		
		//turn on VCO2 pulse, all others off
		switch_byte |= (1<<VCO2_SAW);
		vco_init_cv = &fine_cv;	//VCO2 initi CV currently mapped to fine_cv - need to rename fine_cv to vco2_init_cv
		vco_mix_cv = &vco2_mix_cv;
		vco_pw_cv = &vco2_pw_cv;
		vco_pitch_cv = &vco2_pitch_cv; //need to keep this 0V during initial pitch setting
		reference_count = 19111;
	}
	
	//latch switch data
	DATA_BUS = switch_byte;
	VCO_SW_LATCH_PORT |= (1<<VCO_SW_LATCH);
	//_delay_us(1); //why is this delay here????
	VCO_SW_LATCH_PORT &= ~(1<<VCO_SW_LATCH);
	DATA_BUS = 0;

	PORTF |= (1<<GATE); //turn gate on
	
	//set up timer/counter0 to be clocked by T0 input
	
	TCCR0A |= (1<<CS02) | (1<<CS01) | (1<<CS00) | (1<<WGM01); //clocked by external T0 pin, rising edge + clear timer on compare match
	OCR0A = 1; //output compare register - set to number of periods to be counted. OCR0A needs to be set to (periods_to_be_counted - 1)
	//set OCR0A to 1 here means first ISR interrupt will occur after 2 periods, it is then set to 0 to count only single periods
	//for reasons I don't understand yet, starting with OCR0A set to 0 results in a multi-second delay before first compare match ISR is called
	TIMSK0 |= (1<<OCIE0A); //enable output compare match A interrupt	
	period = 1; //only counting 1 period 
	for (int dac_bit = 13; dac_bit >= 0; dac_bit--) {
	
		init_cv |= (1<<dac_bit);
		
		set_control_voltage(vco_init_cv, init_cv);
		
		
		count_finished = FALSE;
		period_counter = 0;
		//TIMSK0 |= (1<<OCIE0A); //enable output compare match A interrupt
		
		while (count_finished == FALSE) { //need to have a watchdog timer here to escape while loop if it takes too long
			
			update_display(vco + 1, DEC);

			
			set_control_voltage(vco_init_cv, init_cv);
			set_control_voltage(vco_pw_cv, MAX);
			set_control_voltage(&volume_cv, MIN);
			set_control_voltage(&cutoff_cv, MAX);
			set_control_voltage(&sustain_1_cv, MAX);
			set_control_voltage(&sustain_2_cv, MAX); //can't remember is EG1 for VCA or EG2????
			set_control_voltage(vco_mix_cv, MAX);
			set_control_voltage(vco_pitch_cv, 0);	
			
		}
		//remember that as INIT_CV goes up, pitch goes down, so looking for osc_count >= reference_count instead of <= as is the case for normal oscillator tuning
		//similarily, OR no_overflow == FALSE not AND no_overflow == FALSE to clear bits that make initial pitch too low
		if ((osc_count > reference_count)  || (no_overflow == FALSE)) init_cv &= ~(1 << dac_bit);
		no_overflow = TRUE;
		
	}		
	
	//none of these help with clicking when returning from this function and starting to read pots	
	set_control_voltage(&release_1_cv, MIN); //this will hopefully reduce popping after returning from initializing pitch CV
	set_control_voltage(&release_2_cv, MIN);
	set_control_voltage(&cutoff_cv, MIN);
		
	PORTF &= ~(1<<GATE); //turn gate off
	
	TIMSK0 &= ~(1<<OCIE0A); //turn off compare match A interrupt
	
	return init_cv;
	
}

void tune_octave(uint8_t octave, uint8_t vco) {

	uint16_t pitch_reference[12] = { //holds the 2.5MHz counts for each note within an octave independent of octave # being tuned
	    
	PITCH_C,
	PITCH_Cb,
	PITCH_D,
	PITCH_Db,
	PITCH_E,
	PITCH_F,
	PITCH_Fb,
	PITCH_G,
	PITCH_Gb,
	PITCH_A,
	PITCH_Ab,
	PITCH_B
	    
    };
	
	uint8_t period_table[10] = {1, 2, 4, 1, 2, 4, 8, 16, 32, 64}; //the number of  periods that need to be counted for octaves 0-9
	//octaves 0-2 use /64 0.312500 MHz timer/counter1 clock rate
	//octaves 3-9 use /8 2.5 MHz timer/counter1 clock rate
	
	period = period_table[octave]; //set period number to be counted
	
	//to do:
	//disable UART so MIDI data doesn't interrupt tuning
	
	//setup control voltages	
	set_control_voltage(&volume_cv, MIN); //turn volume all the way down
	//turn off all pitch modulation
	set_control_voltage(&pitch_lfo_cv, MIN);
	set_control_voltage(&pitch_eg2_cv, MIN);
	set_control_voltage(&pitch_vco2_cv, MIN);
	//turn off glide
	set_control_voltage(&glide_cv, MIN);
	//turn off all pulse width modulation
	set_control_voltage(&pwm_eg2_cv, MIN);
	set_control_voltage(&pwm_lfo_cv, MIN);
	//turn off all filter modulation
	set_control_voltage(&fil_lfo_cv, MIN);
	set_control_voltage(&fil_eg2_cv, MIN);
	set_control_voltage(&fil_vco2_cv, MIN);
	set_control_voltage(&key_track_cv, MIN);
	//open filter with no resonance
	set_control_voltage(&cutoff_cv, MAX);
	set_control_voltage(&res_cv, MIN);
	//turn off VCA LFO modulation
	set_control_voltage(&amp_lfo_cv, MIN);
	//initialize VCA envelope
	set_control_voltage(&attack_1_cv, MIN);
	set_control_voltage(&decay_1_cv, MIN);
	set_control_voltage(&sustain_1_cv, MAX);
	set_control_voltage(&release_1_cv, MIN);
	//turn off noise
	set_control_voltage(&noise_mix_cv, MIN);

	//these variables hold VCO specific parameters
	uint8_t switch_byte = 0;
	uint8_t vco_number = 0; //vco number to display during octave tuning
	uint16_t init_cv = 0;
	uint16_t *vco_pitch_table = NULL; //pointer to vco pitch table
	struct control_voltage *vco_init_cv;
	struct control_voltage *vco_mix_cv;
	struct control_voltage *vco_pw_cv;
	struct control_voltage *vco_pitch_cv;	

	if (vco == VCO1) { //set up parameters for VCO1 tuning
		//this will change in v1.1 of analog board when comparator is used to generate pulse for T0 pin
		//turn on VCO1 pulse, all others off
		switch_byte |= (1<<VCO1_SAW);
		vco_init_cv = &tune_cv; //VCO1 init CV currently mapped to tune_cv - need to rename tune_cv to vco1_init_cv
		vco_mix_cv = &vco1_mix_cv;
		vco_pw_cv = &vco1_pw_cv;
		vco_pitch_cv = &vco1_pitch_cv; //need to keep this 0V during initial pitch setting
		vco_number = 10;
		init_cv = vco1_init_cv;
		vco_pitch_table = vco1_pitch_table;
		
		
	} else { //set up parameters for VCO2 tuning
		
		//turn on VCO2 pulse, all others off
		switch_byte |= (1<<VCO2_SAW);
		vco_init_cv = &fine_cv;	//VCO2 initi CV currently mapped to fine_cv - need to rename fine_cv to vco2_init_cv
		vco_mix_cv = &vco2_mix_cv;
		vco_pw_cv = &vco2_pw_cv;
		vco_pitch_cv = &vco2_pitch_cv; //need to keep this 0V during initial pitch setting
		vco_number = 20;
		init_cv = vco2_init_cv;
		vco_pitch_table = vco2_pitch_table;
	}
	
	if (octave <= 2) {
		
		//set timer/counter1 to /64 0.3125 MHz
		timer1_clock = (1<<CS11) | (1<<CS10);
		
	} else {
		
		//set timer/counter1 to /8 2.5 MHz
		timer1_clock = (1<<CS11);
		
	}

	//set VCO init offset CV
	set_control_voltage(vco_init_cv, init_cv);
	
	//latch switch data
	DATA_BUS = switch_byte;
	VCO_SW_LATCH_PORT |= (1<<VCO_SW_LATCH);
	//_delay_us(1); //why is this delay here????
	VCO_SW_LATCH_PORT &= ~(1<<VCO_SW_LATCH);
	DATA_BUS = 0;

	PORTF |= (1<<GATE); //turn gate on
	
	TCCR0A |= (1<<CS02) | (1<<CS01) | (1<<CS00) | (1<<WGM01); //clocked by external T0 pin, rising edge + clear timer on compare match
	OCR0A = 1; //output compare register - set to number of periods to be counted. OCR0A needs to be set to (periods_to_be_counted - 1)
	//set OCR0A to 1 here means first ISR interrupt will occur after 2 periods, it is then set to period -1 in output compare ISR
	//for reasons I don't understand yet, starting with OCR0A set to 0 results in a multi-second delay before first compare match ISR is called
	TIMSK0 |= (1<<OCIE0A); //enable output compare match A interrupt
		
	for (int note_number = 0; note_number <= 11; note_number++) 
		{
		
		uint16_t reference_count = pitch_reference[note_number];
		uint16_t osc_pitch_cv = 136 + vco_pitch_table[(octave*12 + note_number) - 1]; //use previous pitch CV as a reference and add 136, which is ~83.333 mV on 14 bit 10V scale. ie. 1/12 of 1V
		//this method is faster than full 14 bit successive approximation, but isn't 'blind', so if there is some wacky scaling at the pitch extremes it could falter
		//will need to test more thoroughly
		for (int dac_bit = 6; dac_bit >= 0; dac_bit--) { //now do successive approximation on 7 LSBs
			osc_pitch_cv = (osc_pitch_cv >> dac_bit) << dac_bit; //clear bits to be set
			osc_pitch_cv |= (1<<dac_bit);

			set_control_voltage(vco_pitch_cv, osc_pitch_cv);
			count_finished = FALSE;
			period_counter = 0;
			

			while (count_finished == FALSE) {
				update_display(vco_number + octave, DEC);	
				//need to have a watchdog timer here to escape while loop if it takes too long
				
				//not sure what's really necessary here - definitely pitch and init_cv, but what else?
				set_control_voltage(vco_pitch_cv, osc_pitch_cv);
				set_control_voltage(vco_init_cv, init_cv);
				set_control_voltage(vco_pw_cv, MAX);
				set_control_voltage(&volume_cv, MIN);//may not be necessary
				set_control_voltage(&cutoff_cv, MAX);
				set_control_voltage(&sustain_1_cv, MAX);
				set_control_voltage(&sustain_2_cv, MAX); //can't remember is EG1 for VCA or EG2????
				set_control_voltage(vco_mix_cv, MAX);
			
			
			}							
				
			//Omar changed this from <= to < which makes sense. <= was an error because if it's equal you don't want to clear the bit
			if ((osc_count < reference_count) && (no_overflow == TRUE)) osc_pitch_cv &= ~(1<<dac_bit);
				
			if (osc_count == reference_count && no_overflow == TRUE) {
				break;	//if you hit the reference count then get out of here		
			}				
			no_overflow = TRUE;
			
			
		}
		
		//will need to make an excpetion for C0 here as its pitch has already been determined by set_vco_init_cv() and so C0 will be 0V
		//for now maybe just start tuning octaves 1 and up
		//vco_pitch_table[octave*12 + note_number] = osc_pitch_cv; //store the note control voltage in the pitch table
		*(vco_pitch_table + (octave*12 + note_number)) = osc_pitch_cv;		
		
		
	}
	
	//TIMSK0 &= ~(1<<OCIE0A); //turn off compare match A interrupt
	
	////setup control voltages
	//set_control_voltage(&volume_cv, MIN); //turn volume all the way down
	////turn off all pitch modulation
	//set_control_voltage(&pitch_lfo_cv, MIN);
	//set_control_voltage(&pitch_eg2_cv, MIN);
	//set_control_voltage(&pitch_vco2_cv, MIN);
	////turn off glide
	//set_control_voltage(&glide_cv, MIN);
	////turn off all pulse width modulation
	//set_control_voltage(&pwm_eg2_cv, MIN);
	//set_control_voltage(&pwm_lfo_cv, MIN);
	////turn off all filter modulation
	//set_control_voltage(&fil_lfo_cv, MIN);
	//set_control_voltage(&fil_eg2_cv, MIN);
	//set_control_voltage(&fil_vco2_cv, MIN);
	//set_control_voltage(&key_track_cv, MIN);
	////open filter with no resonance
	//set_control_voltage(&cutoff_cv, MAX);
	//set_control_voltage(&res_cv, MIN);
	////turn off VCA LFO modulation
	//set_control_voltage(&amp_lfo_cv, MIN);
	////initialize VCA envelope
	//set_control_voltage(&attack_1_cv, MIN);
	//set_control_voltage(&decay_1_cv, MIN);
	//set_control_voltage(&sustain_1_cv, MAX);
	//set_control_voltage(&release_1_cv, MIN);
	////turn off noise
	//set_control_voltage(&noise_mix_cv, MIN);	
	//set_control_voltage(&vco1_mix_cv, MIN);
//
	////now tune VCO2
	//set_control_voltage(&fine_cv, vco2_init_cv);
	//
	////latch switch data
	//DATA_BUS = (1<<VCO2_SAW);
	//VCO_SW_LATCH_PORT |= (1<<VCO_SW_LATCH);
	////_delay_us(1); //why is this delay here????
	//VCO_SW_LATCH_PORT &= ~(1<<VCO_SW_LATCH);
	//DATA_BUS = 0;
//
	//PORTF |= (1<<GATE); //turn gate on
	//
	////TCCR0A |= (1<<CS02) | (1<<CS01) | (1<<CS00) | (1<<WGM01); //clocked by external T0 pin, rising edge + clear timer on compare match
	//
	//OCR0A = 1; //output compare register - set to number of periods to be counted. OCR0A needs to be set to (periods_to_be_counted - 1)
	////set OCR0A to 1 here means first ISR interrupt will occur after 2 periods, it is then set to period -1 in output compare ISR
	////for reasons I don't understand yet, starting with OCR0A set to 0 results in a multi-second delay before first compare match ISR is called
	//TIMSK0 |= (1<<OCIE0A); //enable output compare match A interrupt
	//
	//for (int note_number = 0; note_number <= 11; note_number++)
	//{
		//
		//uint16_t reference_count = pitch_reference[note_number];
		//uint16_t osc_pitch_cv = 136 + vco2_pitch_table[(octave*12 + note_number) - 1];
		//for (int dac_bit = 6; dac_bit >= 0; dac_bit--) {
			//
			//osc_pitch_cv = (osc_pitch_cv >> dac_bit) << dac_bit; //clear bits to be set
			//osc_pitch_cv |= (1<<dac_bit);
			//set_control_voltage(&vco2_pitch_cv, osc_pitch_cv);
			//count_finished = FALSE;
			//period_counter = 0;
			//
//
			//while (count_finished == FALSE) { //need to have a watchdog timer here to escape while loop if it takes too long
				//update_display(20 + octave, DEC);
	//
				//set_control_voltage(&vco2_pitch_cv, osc_pitch_cv);
				//set_control_voltage(&fine_cv, vco2_init_cv);
				//set_control_voltage(&vco2_pw_cv, MAX);
				//set_control_voltage(&volume_cv, MIN);
				//set_control_voltage(&cutoff_cv, MAX);
				//set_control_voltage(&sustain_1_cv, MAX);
				//set_control_voltage(&sustain_2_cv, MAX); //can't remember is EG1 for VCA or EG2????
				//set_control_voltage(&vco2_mix_cv, MAX);
	//
	//
			//}
//
			////}
//
			////if the period timer is less than the reference count and an overflow did not occur then the pitch is too high so clear last bit set
			//if ((osc_count < reference_count) && (no_overflow == TRUE)) osc_pitch_cv &= ~(1<<dac_bit);
			//
			//if (osc_count == reference_count && no_overflow == TRUE) {
				//break;
			//}		
			//no_overflow = TRUE;
		//
		//}
		//
		////will need to make an excpetion for C0 here as its pitch has already been determined by set_vco_init_cv() and so C0 will be 0V
		////for now maybe just start tuning octaves 1 and up
		//vco2_pitch_table[octave*12 + note_number] = osc_pitch_cv; //store the note control voltage in the pitch table			
		//
		//
	//}



	
	PORTF &= ~(1<<GATE); //turn gate off
		
	TIMSK0 &= ~(1<<OCIE0A); //turn off compare match A interrupt
	
	
}