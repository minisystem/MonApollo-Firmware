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

volatile uint8_t compare_match_counter = 0; //diagnostic counter to troubleshoot OCR0A compare match business

//these probably don't need to be volatile - not accessed in interrupt
volatile uint16_t vco1_init_cv = 0;
volatile uint16_t vco2_init_cv = 0;

uint16_t vco1_pitch_table[17] = {0};
uint16_t vco2_pitch_table[17] = {0};

void initialize_voice_for_tuning(void) {
	
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
}	
	
uint16_t set_vco_init_cv(uint8_t vco, uint16_t base_reference) { //should add extra argument here to set reference count for base frequency

	uint16_t init_cv = 0;
	timer1_clock |= (1<<CS11) | (1<<CS10);
	 
	//TO DO: disable UART so MIDI data doesn't interrupt tuning
	
	//setup control voltages	
	initialize_voice_for_tuning();
	
	//these variables hold VCO specific parameters
	uint8_t switch_byte = 0;
	uint16_t reference_count = 0;
	struct control_voltage *vco_init_cv;
	struct control_voltage *vco_mix_cv;
	struct control_voltage *vco_pw_cv;
	struct control_voltage *vco_pitch_cv;

	count_finished = FALSE;
	
	if (vco == VCO1) { //turn on VCO1 pulse
		//turn on VCO1 pulse, all others off
		switch_byte = (1<<VCO1_SAW_LATCH_BIT);
		vco_init_cv = &tune_cv; //VCO1 init CV currently mapped to tune_cv - need to rename tune_cv to vco1_init_cv
		vco_mix_cv = &vco1_mix_cv;
		vco_pw_cv = &vco1_pw_cv;
		vco_pitch_cv = &vco1_pitch_cv; //need to keep this 0V during initial pitch setting
		reference_count = base_reference;//38222; //make this an argument passed to function
		
	} else { //turn on VCO2 pulse	
		//turn on VCO2 pulse, all others off
		switch_byte = (1<<VCO2_SAW_LATCH_BIT);
		vco_init_cv = &fine_cv;	//VCO2 initi CV currently mapped to fine_cv - need to rename fine_cv to vco2_init_cv
		vco_mix_cv = &vco2_mix_cv;
		vco_pw_cv = &vco2_pw_cv;
		vco_pitch_cv = &vco2_pitch_cv; //need to keep this 0V during initial pitch setting
		reference_count = base_reference;
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
	//for reasons I don't understand yet, starting with OCR0A set to 0 results in a multi-second delay before first compare match ISR is called ***ACTUALLY, THIS MAY BE RELATED TO LEAVING Timer0 RUNNING. COULD TRY TO SET OCR0A TO 0 NOW and SEE IF IT WORKS
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
	set_control_voltage(&volume_cv, MIN);
		
	PORTF &= ~(1<<GATE); //turn gate off
	
	TIMSK0 &= ~(1<<OCIE0A); //turn off compare match A interrupt
	TCCR0A = 0; //turn off timer0 period timer
	
	return init_cv;
	
}



void tune_8ths(uint8_t vco) {
	


	struct pitch_reference {
	
	
		uint8_t period;
		uint16_t count;
	
		};

	struct pitch_reference reference[16] = 
		{

			{	1	,	15169	}, //start with MIDI note 16 (20.6 Hz). Calibrate init_cv for both oscillators to be MIDI note 8 (12.9 Hz), so when vco_pitch_cv = 0, frequency = 12.9 Hz
			{	2	,	19111	},
			{	2	,	12039	},
			{	1	,	30337	},
			{	1	,	19111	},
			{	2	,	24079	},
			{	2	,	15169	},
			{	4	,	19111	},
			{	4	,	12039	},
			{	8	,	15169	},
			{	8	,	9556	},
			{	16	,	12039	},
			{	16	,	7584	},
			{	32	,	9556	},
			{	32	,	6020	},
			{	64	,	7584	} //end with "MIDI note" 136 - this allows octave switching to go all the way up to and beyond 21 KHz
	
		};

	
		//to do:
		//disable UART so MIDI data doesn't interrupt tuning
	
		//setup control voltages	
		initialize_voice_for_tuning();

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

			//turn on VCO1 SAW, all others off
			switch_byte = (1<<VCO1_SAW_LATCH_BIT);
			vco_init_cv = &tune_cv; //VCO1 init CV currently mapped to tune_cv - need to rename tune_cv to vco1_init_cv
			vco_mix_cv = &vco1_mix_cv;
			vco_pw_cv = &vco1_pw_cv;
			vco_pitch_cv = &vco1_pitch_cv; //need to keep this 0V during initial pitch setting
			vco_number = 10; //allows second digit to display VCO being tuned
			init_cv = vco1_init_cv;
			vco_pitch_table = vco1_pitch_table;
		
		
		} else { //set up parameters for VCO2 tuning
		
			//turn on VCO2 SAW, all others off
			switch_byte = (1<<VCO2_SAW_LATCH_BIT);
			vco_init_cv = &fine_cv;	//VCO2 initi CV currently mapped to fine_cv - need to rename fine_cv to vco2_init_cv
			vco_mix_cv = &vco2_mix_cv;
			vco_pw_cv = &vco2_pw_cv;
			vco_pitch_cv = &vco2_pitch_cv; //need to keep this 0V during initial pitch setting
			vco_number = 20; //allows second digit to display VCO being tuned
			init_cv = vco2_init_cv;
			vco_pitch_table = vco2_pitch_table;
		}
	


		//set VCO init offset CV
		set_control_voltage(vco_init_cv, init_cv);
		//set_control_voltage(vco_pitch_cv, 8192);
	
		//latch switch data
		DATA_BUS = switch_byte;
		VCO_SW_LATCH_PORT |= (1<<VCO_SW_LATCH);
		//_delay_us(1); //why is this delay here????
		VCO_SW_LATCH_PORT &= ~(1<<VCO_SW_LATCH);
		DATA_BUS = 0;

		PORTF |= (1<<GATE); //turn gate on
			
		period = 1; //need to initialize to minimum period number here
		
		compare_match_counter = 0;	
		for (int note_number = 0; note_number <= 15; note_number++) 
			{
			period = reference[note_number].period;
			//period timer needs to be initialized here and turned off after each note's SAR to prevent glitching caused by leaving timer0 running
			TCCR0A |= (1<<CS02) | (1<<CS01) | (1<<CS00) | (1<<WGM01); //clocked by external T0 pin, rising edge + clear timer on compare match
			OCR0A = 1; //output compare register - set to number of periods to be counted. OCR0A needs to be set to (periods_to_be_counted - 1) ***COULD PROBABLY CHANGE THIS TO 0 NOW*** - NOPE. NEEDS TO BE 1!!!
			TIMSK0 |= (1<<OCIE0A); //enable output compare match A interrupt
			TCNT0 = 0; //make sure timer/counter0 is actually 0. 
			
			if (note_number <= 2) {
	
				//set timer/counter1 to /64 0.3125 MHz
				timer1_clock = (1<<CS11) | (1<<CS10);
	
			} else {
	
				//set timer/counter1 to /8 2.5 MHz
				timer1_clock = (1<<CS11);
	
			}
			//the following should be moved to its own function as it is duplicated in the init_cv function. Something like tune_note(*cv, period, reference_count), where *cv points to CV that needs to be calculated`
			uint16_t reference_count = reference[note_number].count;
			uint16_t osc_pitch_cv = 0;
			for (int dac_bit = 13; dac_bit >= 0; dac_bit--) { //now do successive approximation
				
				osc_pitch_cv |= (1<<dac_bit);

				set_control_voltage(vco_pitch_cv, osc_pitch_cv);
				count_finished = FALSE;
				period_counter = 0;
			
				
				while (count_finished == FALSE) {
					//update_display(vco_number + period + (compare_match_counter>>4)*100, DEC);
					update_display(vco_number*100 + period, DEC);//
					//value_to_display = TCNT0;
					//update_display(value_to_display, DEC);	
					//need to have a watchdog timer here to escape while loop if it takes too long
				
					//not sure what's really necessary here - definitely pitch and init_cv, but what else?
					set_control_voltage(vco_init_cv, init_cv);
					set_control_voltage(vco_pitch_cv, osc_pitch_cv);
					
					//set_control_voltage(vco_pw_cv, MAX); //not necessary as SAW is being used to clock comparator
					set_control_voltage(&volume_cv, MIN);//only necessary for first 2 octaves that use lower frequency reference clock
					set_control_voltage(&cutoff_cv, MAX);
					set_control_voltage(&sustain_1_cv, MAX);
					set_control_voltage(&attack_1_cv, MIN); //keep attack at minimum
					//set_control_voltage(&sustain_2_cv, MAX); //can't remember is EG1 for VCA or EG2????
					set_control_voltage(vco_mix_cv, MAX);
			
			
				}							
				
				//Omar changed this from <= to < which makes sense. <= was an error because if it's equal you don't want to clear the bit
				if ((osc_count <= reference_count) && (no_overflow == TRUE)) osc_pitch_cv &= ~(1<<dac_bit);
				
				if (osc_count == reference_count && no_overflow == TRUE) {
					break;	//if you hit the reference count then get out of here		
				}				
				no_overflow = TRUE;
			
			
			}
		
			//vco_pitch_table[octave*12 + note_number] = osc_pitch_cv; //store the note control voltage in the pitch table
			*(vco_pitch_table + (note_number+1)) = osc_pitch_cv;		
			
			//need to turn timer off here. This seems to have stopped periodic glitching of first note of first VCO tuned.
			TIMSK0 &= ~(1<<OCIE0A); //turn off timer0 compare match A interrupt
			TCCR0A = 0; //turn off timer0
		}
	

	
		PORTF &= ~(1<<GATE); //turn gate off
		
		//TIMSK0 &= ~(1<<OCIE0A); //turn off compare match A interrupt
	
	
	}	
	
uint16_t interpolate_pitch_cv(uint8_t note, uint16_t *pitch_table) {
	
	uint8_t pitch_index = note>>3;
	uint8_t delta_note = note - pitch_index*8; //will range from 0 to 7
		
	uint16_t y0 = pitch_table[pitch_index -1];
	uint16_t y1 = pitch_table[pitch_index];
	
	uint16_t interpolated_pitch_cv = y0 + (((y1 - y0)*delta_note)>>3); //mind order of operations here: + is evaluated before >>	
	
	return interpolated_pitch_cv;
	
}