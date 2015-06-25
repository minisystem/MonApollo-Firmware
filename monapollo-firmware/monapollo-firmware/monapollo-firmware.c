/*
 * monapollo_firmware.c
 *
 * Created: 2015-06-23 5:27:02 PM
 *  Author: jeff
 */ 

#define F_CPU 20000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "spi.h"
#include "dac.h"
#include "display.h"
#include "port_map.h"
#include "led_map.h"
#include "switch_map.h"

//define POTMUX_EN bits
#define POTMUX_EN0	PH6
#define POTMUX_EN1	PH7

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
#define LFO_RATE		1 //U2 pot demultiplexer bit 10 //bit 11 is ARP_RATE, which is not used to set a DAC 
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

#define GATE PF1 //define gate output

//counter for switch debouncing
volatile uint8_t switch_timer = 0;

volatile uint8_t place = 0; //digit place for LED display

volatile uint16_t adc_previous = 0;
volatile uint16_t adc_value = 0;
volatile uint16_t tune_offset = 0; //fine tune offset to display 

volatile uint16_t value_to_display = 747; //global to hold display value

volatile uint16_t dac_channel[] = { //array to store 8 14 bit DAC values, which are determined by ADC readings of 8 pots
	
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

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

void setup_midi_usart(void)
{
    uint16_t ubbr_value = 39; //20MHz/(16*31250 BAUD) - 1
    //write ubbr_value to H and L UBBR1 registers:
    UBRR0L = (unsigned char) ubbr_value;
    UBRR0H = (unsigned char) (ubbr_value >> 8);
	
	UCSR0B = (1<<RXEN0)|(1<<TXEN0) | (1<<RXCIE0);
	//UCSR0C |= (0<<UMSEL0)|(0<<UMSEL01)|(0<<UPM01)|(0<<UPM00)|(0<<USBS0)|(0<<UCSZ02)|(1<<UCSZ01)|(1<<UCSZ00);  	
}


volatile uint8_t digit[] = {
	ONES,
	TENS,
	HUNDS,
	THOUS,
};

void setup_adc(void)
{
	//ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0); //set ADC clock to 156.25 KHz for 20 MHz clock
	//ADCSRA |= (1<<ADPS2) | (1<<ADPS1); //set ADC clock to 312.5 KHz for 20 MHz clock
	ADCSRA |= (1<<ADPS2);// | (1<<ADPS0); //set ADC clock to 1.25 MHz for 20 MHz clock
	ADMUX |= (1<<REFS0); //set ADC reference to AVCC (+5V)	
	
	DIDR0 |= 0x01; //disable digital input buffer for ADC0
	
	ADCSRA |= (1<<ADEN); //enable ADC	
}

ISR (USART_RX_vect) { // USART receive interrupt
	 
	 //if (UDR0 == 248) {
		 //PORTB ^= (1<<ARP_SYNC_LED);
		 //return;
	 //}		  
	 value_to_display = UDR0;
	 uint8_t status_byte = value_to_display >> 4;
	 
	 if ((status_byte >> 3) == 1) //if it's a note on or off event, handle it:
	 { 
		 if ((status_byte >> 0) & 1) {PORTF |= (1<<GATE);} else {PORTF &= ~(1<<GATE);}
		//PORTF ^= (1<<GATE);	 
     }	else if (value_to_display == 0) {PORTF &= ~(1<<GATE);}
	  
	
}

ISR (TIMER2_OVF_vect) { //main scanning interrupt handler
	
	display_dec(value_to_display, digit[place]);
	
			
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
		
	//do SPI read/write every 5 interrupts (16.5 ms)
	if (switch_timer++ == 5)
	{
		switch_timer = 0;
		update_spi();	
			  	
	}
	
	

	//increment digit display place
	if (place++ == 3) //post increment
	{
		place = 0;
	}
	

	
}	




int main(void)
{
	//turn off JTAG so all outputs of PORTC can be used
	MCUCR = (1<<JTD);
	MCUCR = (1<<JTD);
		
	//SET PORTB PIN 7 (PB7) as OUTPUT
	DDRB |= (1<<ARP_SYNC_LED);
	
	DDRF |= (1<<GATE); //set gate as output
	//PORTF |= (1<<GATE); //turn gate on for testing
	
	setup_spi(); 
	
	DDRH |= (1<<POTMUX_EN0) | (1<<POTMUX_EN1); //set POTMUX_EN pins as outputs
	POT_MUX |= (1<<POTMUX_EN0) | (1<<POTMUX_EN1); //set POTMUX_EN pins HIGH (active LOW)
	//POT_MUX |= (1<<POTMUX_EN1);
	
	//set up LED display
	DDRA |= 0b11111111; //set all lines or DATA_BUS to outputs
	DATA_BUS |= 0b11111111; //set all DATA_BUS lines to HIGH (cathodes OFF)
	DDRH |= (1<<DISP_CATHODE_LATCH) | (1<<DISP_ANODE_LATCH); //set display latches to outputs
	DISPLAY_PORT &= ~(1<<DISP_ANODE_LATCH | 1<< DISP_CATHODE_LATCH); //set DISP latches to LOW (inactive)
	
	//set up switch port
	DDRF &= ~(1<<ISW8_SW); //set ISW8_SW pin as input
	
	//setup ADC
    setup_adc();		
	//setup DAC
	setup_dac();
	//setup MIDI USART
	setup_midi_usart();
	
	//set up main timer interrupt
	//this generates the main scanning interrupt
	TCCR2A |= (1<<CS22) | (1<<CS21); //Timer2 20MHz/256 prescaler
	TIMSK2 |= (1<<TOIE2); //enable Timer2 overflow interrupt over flows approx. every 3ms	
	sei(); //enable global interrupts

	while(1)
	{	
		//PORTB |= (1<<ARP_SYNC_LED);
	}
}