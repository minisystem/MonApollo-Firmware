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
//#include "dac.h"
#include "display.h"
#include "display_map.h"
#include "port_map.h"
//#include "led_map.h"
//#include "switch_map.h"

//ARP_SYNC LED driven directly from AVR
#define ARP_SYNC_LED	PB7



//EG2 polarity pin on PORTJ
#define EG2_POL			PJ4

//define analog switch latch (VCO waveform switching)
#define VCO_SW_LATCH		PJ6
//define LFO waveform switch latch (not yet implemented in hardware)
#define LFO_SW_LATCH		PJ5

//define DAC bits
#define DAC_WR			PG0
#define DAC_RS			PG1
#define DAC_MUX_EN0		PH0 //DG408 enable is active HIGH
#define DAC_MUX_EN1		PH1
#define DAC_MUX_EN2		PH2
#define DAC_MUX_EN3		PH3

//define LED bits - NEED TO CHANGE THESE TO BIT POSITIONS 0-7
#define ISW12_LED			0b00000100
#define ISW11_LED			0b10000000
#define ISW8_LED			7 //B MOD
#define ISW4_LED			1 //SYNC U16
#define ISW1_LED			2 //VCO1 SAW U16
#define ISW2_LED			3 //VCO1 TRI U16
#define ISW3_LED			0 //VCO1 PULSE U16
#define ISW5_LED			4 //VCO2 SAW U16
#define ISW6_LED			5 //VCO2 TRI U16
#define ISW7_LED			6 //VCO2 PULSE U16

//define SPI switch bits - NEED TO CHANGE THESE TO BIT POSITIONS 0-7
#define ISW12_SW			0b00100000
#define ISW13_SW			0b01000000

//these switch bits are defined asin bit order coming in on SPI bus (same as data line connected to 74HC165)
#define ISW1_SW				2 //VCO1 SAW U14
#define ISW2_SW				1 //VCO1 TRI U14
#define ISW3_SW				0 //VCO1 PULSE U14
#define ISW4_SW				7 //SYNC U14
#define ISW5_SW				4 //VCO2 SAW U14
#define ISW6_SW				5 //VCO2 TRI U14
#define ISW7_SW				6 //VCO2 PULSE U14
//define direct MCU input switch bits
#define ISW8_SW				PF2

//SPI switch latch
#define SPI_SW_LATCH		(1<<PB5)



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

//switch flags
//ultimately combine these into a single byte and do bit manipulations to determine switch states
volatile uint8_t ISW12_SW_ON = 0; //flag for ISW12 switch
volatile uint8_t ISW13_SW_ON = 0; //flag for ISW13 switch
volatile uint8_t ISW4_SW_ON = 0;  //flag for ISW4 switch
volatile uint8_t ISW8_SW_ON = 0; //flag for ISW8 switch (direct bus into MCU)

//debounce switch state flags
//again, combine into a single byte and do bit manipulations to determine previous switch states
//flags for direct (into MCU) input switches
volatile uint8_t previous_sw_state = 0;
volatile uint8_t current_sw_state = 0;
//flags for SPI input switches (currently only 1 SPDR byte - will need to make these variables wider or have multiple flag bytes for each successive SPI byte)
volatile uint8_t spi_sw_current_state = 0;
volatile uint8_t spi_sw_previous_state = 0;
//switch state holder
volatile uint8_t sw_latch_five = 0; //U16 5th switch latch in SPI chain
volatile uint8_t sw_latch_four = 0;
volatile uint8_t sw_latch_three = 0;
volatile uint8_t sw_latch_two = 0;
volatile uint8_t sw_latch_one = 0;

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
void set_dac(uint8_t dac_mux_address, uint8_t channel, uint16_t value)
{
	
	
	DAC_BUS_LOW = value & 0b00000011111111; //mask top 6 MSBs to set low byte
	
	DAC_BUS_HIGH = value >> 8; //shift away bottom LSBs to set high byte
	
	DAC_CTRL &= ~(1<<DAC_WR); //write DATA
	DAC_CTRL |= (1<<DAC_WR);
	
	DATA_BUS = channel; //set channel for DG408 multiplexer output
	//uint8_t dac_mux_address;
	//use switch:case statement here to set appropriate DAC_MUC_EN bit. Could probably do some
	//bit shifting to derive it from channel number too (0-7: DAC_MUX_EN0, 8-15: DAC_MUX_EN1, 9-23: DAC_MUX_EN2, 24-31: DAC_MUX_EN3
	//if (channel < 8)
	//{
		
		//dac_mux_address = DAC_MUX_EN0;
	//} else {
		//
		//dac_mux_address = DAC_MUX_EN1;
	//}
	_delay_us(2); //AD5556 DAC has 0.5 us settling time. 1 us wasn't long enough for transitions from 10V to 0V
	DAC_MUX |= (1<<dac_mux_address); //enable multiplexer
	_delay_us(10); //wait for S&H cap to charge - need to figure out how to do this more time efficiently
	DAC_MUX &= ~(1<<dac_mux_address); //disable multiplexer
	
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
		
		
		SPI_PORT |= SPI_SW_LATCH;
		
		//SHIFT 5th BYTE
		SPDR =  
		((sw_latch_five >> ISW4_SW) & 1) << ISW4_LED |
		((sw_latch_five >> ISW1_SW) & 1) << ISW1_LED |
		((sw_latch_five >> ISW2_SW) & 1) << ISW2_LED |
		((sw_latch_five >> ISW3_SW) & 1) << ISW3_LED |
		((sw_latch_five >> ISW5_SW) & 1) << ISW5_LED |
		((sw_latch_five >> ISW6_SW) & 1) << ISW6_LED |
		((sw_latch_five >> ISW7_SW) & 1) << ISW7_LED |
		ISW8_SW_ON << ISW8_LED; 
		
		while (!(SPSR & (1<<SPIF)));
		
		//Now read SPDR for switch data shifted in from 74XX165 U14
		spi_sw_current_state = SPDR;
		
		spi_sw_current_state ^= spi_sw_previous_state;
		spi_sw_previous_state ^= spi_sw_current_state;
		spi_sw_current_state &= spi_sw_previous_state;
		
		//toggle switch state 		

		if (spi_sw_current_state & (1<<ISW1_SW)) sw_latch_five ^= (1 << ISW1_SW);
		if (spi_sw_current_state & (1<<ISW2_SW)) sw_latch_five ^= (1 << ISW2_SW);					
		if (spi_sw_current_state & (1<<ISW3_SW)) sw_latch_five ^= (1 << ISW3_SW);
		if (spi_sw_current_state & (1<<ISW4_SW)) sw_latch_five ^= (1 << ISW4_SW);	
		if (spi_sw_current_state & (1<<ISW5_SW)) sw_latch_five ^= (1 << ISW5_SW);
		if (spi_sw_current_state & (1<<ISW6_SW)) sw_latch_five ^= (1 << ISW6_SW);
		if (spi_sw_current_state & (1<<ISW7_SW)) sw_latch_five ^= (1 << ISW7_SW);
		
		//SHIFT 4th BYTE
		SPDR = 0; //no LEDs connected in current test set up
		while (!(SPSR & (1<<SPIF)));
		//Now read SPDR for switch data shifted in from 74XX165 (U9)
		//check if ISW12_SW bit is set
		if (SPDR >> 5 & 1)
		{
			ISW12_SW_ON = 1;
		}
		else
		{
			ISW12_SW_ON = 0;
		}
		//check if ISW13_SW bit is set
		if (SPDR >> 6 & 1)
		{
			ISW13_SW_ON = 1;
		}
		else
		{
			ISW13_SW_ON = 0;
		}
		
		//SHIFT 3th BYTE
		SPDR = 0;
		while (!(SPSR & (1<<SPIF)));

		//SHIFT 2th BYTE
		SPDR = 0;
		while (!(SPSR & (1<<SPIF)));
		
		//SHIFT 1st BYTE
		//SPDR = (ISW12_SW_ON << 2) | ISW11_LED; //TURN ON ISW12 (if ISW12_SW is ON) and ISW11 LEDs, both on 74XX595 U8, first shift register in chain
		SPDR = (ISW12_SW_ON <<2) | (ISW13_SW_ON << 7); //turn on ISW12 if ISW12_SW is ON, turn ISW11 (MSB of first shift register chain) if ISW13_SW is ON
		//Wait for SPI shift to complete
		while (!(SPSR & (1<<SPIF)));
		
		//Toggle LED_LATCH to shift data to 74HC595 shift register outputs
		
		SPI_LATCH_PORT &= ~LED_LATCH;
		SPI_LATCH_PORT |= LED_LATCH;
		
		//clear SPI_SW_LATCH
		SPI_PORT &= ~SPI_SW_LATCH;	
		//this toggle code works, but I haven't figured out how it works
		//source: http://forum.allaboutcircuits.com/threads/help-with-programming-uc-toggle-led-using-one-switch.51602/
		current_sw_state = SWITCH_PORT;
		current_sw_state ^= previous_sw_state;
		previous_sw_state ^= current_sw_state;
		current_sw_state &= previous_sw_state;
		
		if (current_sw_state & (1<<ISW8_SW)) 
		{
			ISW8_SW_ON ^= 1 << 0; //toggle switch state
		}
		
		//update analog switch latch:
		VCO_SW_LATCH_PORT &= ~(1<<VCO_SW_LATCH);
		//enable output on VCO analog switch latch:
		//switch latch: 7: B TRI 6: B SAW 5: B PULSE 4: B MOD 3: SYNC 2: A TRI 1: A PULSE 0: A SAW
		DATA_BUS =
		((sw_latch_five >> ISW4_SW) & 1) << 3 |
		((sw_latch_five >> ISW1_SW) & 1) << 0 |
		((sw_latch_five >> ISW2_SW) & 1) << 2 |
		((sw_latch_five >> ISW3_SW) & 1) << 1 |
		((sw_latch_five >> ISW5_SW) & 1) << 6 |
		((sw_latch_five >> ISW6_SW) & 1) << 7 |
		((sw_latch_five >> ISW7_SW) & 1) << 5 |
		ISW8_SW_ON << 4;
		VCO_SW_LATCH_PORT |= (1<<VCO_SW_LATCH);
		_delay_us(1);
		VCO_SW_LATCH_PORT &= ~(1<<VCO_SW_LATCH);
		DATA_BUS = 0;			  	
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
	
	//SET SPI_DATA_OUT and SPI_CLK and SPI_SW_LATCH pins as outputs
	//also set Slave Select (PB0) as output just to ensure it doesn't interfere with SPI communication (currently floating)
	//ACTUALLY, Slave Select ***MUST*** be set as output. Leaving it floating without setting its data direction bit breaks SPI!
	DDRB |= (SPI_DATA_OUT | SPI_CLK | SPI_SW_LATCH |(1<<PB0));
	
	//SET SPI_EN and LED_LATCH and VCO_SW_LATCH and EG2_POL pins as outputs
	DDRJ |= (SPI_EN | LED_LATCH | (1<<VCO_SW_LATCH) | (1<<EG2_POL));
	
	//SET SPI_DATA_OUT and SPI_CLK and SPI_SW_LATCH outputs LOW
	SPI_PORT &= ~(SPI_DATA_OUT | SPI_CLK | SPI_SW_LATCH);
	
	//SET SPI_EN LOW (active) and LED_LATCH LOW (active)
	SPI_LATCH_PORT &= ~(SPI_EN | LED_LATCH);
	
	//SET UP SPI
	SPCR = (1<<SPE) | (1<<MSTR); //Start SPI as MASTER
	
	//Pull LED_LATCH LOW
	SPI_LATCH_PORT &= ~LED_LATCH;
	
	////SHIFT DATA IN TO LIGHT ISW12
	//SPDR = 0b11111111; 
	////Wait for SPI shift to complete
	//while (!(SPSR & (1<<SPIF)));
	
	//Toggle LED_LATCH to shift data to 74HC595 shift register outputs
	
	SPI_LATCH_PORT &= ~LED_LATCH;
	SPI_LATCH_PORT |= LED_LATCH;
	
	//set EG2 POL
	EG2_POL_PORT &= ~(1 << EG2_POL); //0 for normal, 1 for inverted
	
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
	//setup ADC, free running for now. Not sure if this is the way it should be done. Look into benefits of one-shot ADC
    setup_adc();	
	
	//setup DAC
	setup_dac();
	
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