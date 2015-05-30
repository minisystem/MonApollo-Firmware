/*
 * Analog_Board_DAC_Test.cpp
 *
 * Created: 2015-05-11 2:23:26 PM
 *  Author: jeff

 
 *Board Evaluation
 ==========================================================
 
 DONE:
 ==============================================================================

 
DAC setup is working. Value is determined by reading 10 bit pot value
DAC MULTIPLEXING:
-Only U13/U14 DG408 VDAC multiplexer and associated S&Hs is installed
-Read 16 pot values and assigns those values to each of the 8 DAC multiplexer channels

SPI LED driving and switch reading is done but only in a rudimentary fashion for testing
Leave it alone as much as possible and try to get LED 7-segment display working with
interrupt-based updates whilst keeping SPI working

Basic testing of 7-segment display is done. Cathode and anode latches and databus can
be used to light individual segments.

Main scanning interrupt (~3.3ms) implemented and SPI and LED display driving code
moved into main interrupt handler routine.

dispay_DEC function can now take any value between 0 and 9999 and display it on display
cycling through 4 places, anode 1 to 4

ADC operates with 1X and 16X oversampling, depending on button press. Seems a pretty noisy. Most pot demuxer
inputs are floating because only 2 pots are soldered on board. This could be a source of noise.
 
 TO DO:
 ==============================================================================
 
*VCO waveform switching
	
*Handle decimal points on LED display
*break up code into header files
 
 ==============================================================================
 
 COMMENTS:
 ==============================================================================
 
 SPI_CLK looks funny on scope. Probing any SCK pin on 165 or 595 with scope probe
 on 1X setting causes ISW12 LED to light as if ISW12 SW had been pressed. Switching
 probe to 10X doesn't cause this problem. Must be a problem with high speed clock
 and probe impedance. SPI clock is at default - currently don't know what speed it 
 is running at.
 
 ==============================================================================
 */

#define F_CPU 20000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//ARP_SYNC LED driven directly from AVR
#define ARP_SYNC_LED	PB7

//SPI pins
#define SPI_DATA_OUT	(1<<PB2)
#define SPI_DATA_IN		PB3
#define SPI_CLK			(1<<PB1)

//SPI EN pin on PORTJ
#define SPI_EN			(1<<PJ2)

//LED latch pin on PORTJ
#define LED_LATCH		(1<<PJ3)

//define analog switch latch (VCO waveform switching)
#define VCO_SW_LATCH		PJ6
//define LFO waveform switch latch (not yet implemented in hardware)
#define LFO_SW_LATCH		PJ5

//define PORTS
#define SPI_PORT			PORTB
#define SPI_LATCH_PORT		PORTJ
#define VCO_SW_LATCH_PORT	PORTJ
#define DATA_BUS			PORTA
#define DISPLAY_PORT		PORTH
#define DAC_BUS_LOW			PORTD
#define DAC_BUS_HIGH		PORTC
#define DAC_CTRL			PORTG
#define DAC_MUX				PORTH
#define POT_MUX				PORTH
#define SWITCH_PORT			PINF //direct reading by MCU of some switches occurs on this port

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
#define ISW8_LED			0b10000000 //B MOD
#define ISW4_LED			0b00000010 //SYNC U16
#define ISW1_LED			0b00000100 //VCO1 SAW U16
#define ISW2_LED			0b00001000 //VCO1 TRI U16
#define ISW3_LED			0b00000001 //VCO1 PULSE U16
#define ISW5_LED			0b00010000 //VCO2 SAW U16
#define ISW6_LED			0b00100000 //VCO2 TRI U16
#define ISW7_LED			0b01000000 //VCO2 PULSE U16

//define SPI switch bits - NEED TO CHANGE THESE TO BIT POSITIONS 0-7
#define ISW12_SW			0b00100000
#define ISW13_SW			0b01000000

#define ISW1_SW				0b00000100 //VCO1 SAW U14
#define ISW2_SW				0b00000010 //VCO1 TRI U14
#define ISW3_SW				0b00000001 //VCO1 PULSE U14
#define ISW4_SW				0b10000000 //SYNC U14
#define ISW5_SW				0b00010000 //VCO2 SAW U14
#define ISW6_SW				0b00100000 //VCO2 TRI U14
#define ISW7_SW				0b01000000 //VCO2 PULSE U14
//define direct MCU input switch bits
#define ISW8_SW				PF2

//SPI switch latch
#define SPI_SW_LATCH		(1<<PB5)

//LED 7-segment display latches
#define DISP_ANODE_LATCH	PH5
#define DISP_CATHODE_LATCH	PH4

//define digit anodes
#define ONES	0b00001000
#define TENS	0b00000001
#define HUNDS	0b00000010
#define THOUS	0b00000100

//define digit cathodes (current sink, active low)
#define a		0b00000100
#define b		0b00000001
#define c		0b00010000
#define d		0b01000000
#define e		0b10000000
#define f		0b00000010
#define g		0b00001000
#define dp		0b00100000

//define decimal digits
#define ZERO	(a | b | c | d | e | f)
#define ONE		(b | c)
#define TWO		(a | b | d | e | g)
#define THREE	(a | b | c | d | g)
#define FOUR	(b | c | f | g)
#define FIVE	(a | c | d | f | g)
#define SIX		(a | c | d | e | f | g)
#define SEVEN	(a | b | c)
#define EIGHT	(a | b | c | d | e | f | g)
#define NINE	(a | b | c | d | f | g)

//define hex digits
#define A		(a | b | c | e | f | g)
#define B		(c | d | e | f | g)
#define C		(a | d | e | f)
#define D		(b | c | d | e | g)
#define E		(a | d | e | f | g)
#define F		(a | e | f | g)

//define POTMUX_EN bits
#define POTMUX_EN0	PH6
#define POTMUX_EN1	PH7

#define GATE PF1 //define gate output

volatile uint8_t ISW12_SW_ON = 0; //flag for ISW12 switch
volatile uint8_t ISW13_SW_ON = 0; //flag for ISW13 switch
volatile uint8_t ISW4_SW_ON = 0;  //flag for ISW4 switch
volatile uint8_t ISW8_SW_ON = 0; //flag for ISW8 switch (direct bus into MCU)

volatile uint8_t place = 0; //digit place for LED display

volatile uint16_t adc_previous = 0;
volatile uint16_t adc_value = 0;
volatile uint16_t tune_offset = 0; //fine tune offset to display 

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
void setupDAC(void) //set up DAC
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
void set_dac(uint8_t channel, uint16_t value)
{
	
	
	DAC_BUS_LOW = value & 0b00000011111111; //mask top 6 MSBs to set low byte
	
	DAC_BUS_HIGH = value >> 8; //shift away bottom LSBs to set high byte
	
	DAC_CTRL &= ~(1<<DAC_WR); //write DATA
	DAC_CTRL |= (1<<DAC_WR);
	
	DATA_BUS = channel; //set channel for DG408 multiplexer output
	uint8_t dac_mux_address;
	if (channel < 8)
	{
		dac_mux_address = DAC_MUX_EN0;
	} else {
		
		dac_mux_address = DAC_MUX_EN1;
	}
	_delay_us(2); //AD5556 DAC has 0.5 us settling time. 1 us wasn't long enough for transitions from 10V to 0V
	DAC_MUX |= (1<<dac_mux_address); //enable multiplexer
	_delay_us(10); //wait for S&H cap to charge - need to figure out how to do this more efficiently
	DAC_MUX &= ~(1<<dac_mux_address); //disable multiplexer
	
}


void display_DEC(uint16_t number, uint8_t digit)
{
	uint8_t DEC[] = {
		//this is the 7-segment display encoding table
		
		ZERO,
		ONE,
		TWO,
		THREE,
		FOUR,
		FIVE,
		SIX,
		SEVEN,
		EIGHT,
		NINE,
		
		
	};
	
	//clear cathode bits
	DATA_BUS = 0xFF; //set bits for cathode (current sinks, active LOW)
	//latch data to cathode lines
	DISPLAY_PORT |= (1<<DISP_CATHODE_LATCH);
	DISPLAY_PORT &= ~(1<<DISP_CATHODE_LATCH);
	
	//set anode bit
	DATA_BUS = digit;
	//latch data to anode lines
	DISPLAY_PORT |= (1<<DISP_ANODE_LATCH);
	DISPLAY_PORT &= ~(1<<DISP_ANODE_LATCH);

	
	//determine cathode byte based on digit to display
	uint8_t cathode_byte;
	
	switch(digit) {
		
		case ONES:
		cathode_byte = DEC[(number % 10)]; //print first decimal digit
		break;
		
		case TENS:
		cathode_byte = DEC[((number % 100) / 10)]; //print second decimal digit
		break;
		
		case HUNDS:
		cathode_byte = DEC[((number % 1000) / 100)]; //print third decimal digit
		break;
		
		case THOUS:
		cathode_byte = DEC[((number % 10000) / 1000)]; //print fourth decimal digit
		break;
		
	}
	
	//set cathode byte
	DATA_BUS = ~(cathode_byte); //set bits for cathode (current sinks, active LOW)
	//latch data to cathode lines
	DISPLAY_PORT |= (1<<DISP_CATHODE_LATCH);
	DISPLAY_PORT &= ~(1<<DISP_CATHODE_LATCH);
	
	//DATA_BUS = 0; //clear DATA_BUS before return
}

volatile uint8_t digit[] = {
	ONES,
	TENS,
	HUNDS,
	THOUS,
};

void setupADC(void)
{
	//ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0); //set ADC clock to 156.25 KHz for 20 MHz clock
	//ADCSRA |= (1<<ADPS2) | (1<<ADPS1); //set ADC clock to 312.5 KHz for 20 MHz clock
	ADCSRA |= (1<<ADPS2);// | (1<<ADPS0); //set ADC clock to 1.25 MHz for 20 MHz clock
	ADMUX |= (1<<REFS0); //set ADC reference to AVCC (+5V)
	
	//MUX2:0 is 000 by default in ADMUX
	//ADMUX &= ~(1<<MUX0); //set ADC multiplexer to read ADC0 (PF0 on PORTF, pin 97)
	
	//ADCSRA |= (1<<ADATE); //set ADC in free running mode
	
	DIDR0 |= 0x01; //disable digital input buffer for ADC0
	
	ADCSRA |= (1<<ADEN); //enable ADC
	
	//ADCSRA |= (1<<ADSC); //start ADC
	
	////do an initial read to set adc_previous	
	//DATA_BUS = 0b00000111; //select Y7 (VR2 POT)
	//PORTH &= ~(1<<POTMUX_EN0); //clear POTMUX_EN0 to select input Y7
	//ADCSRA |= (1<<ADSC); //start ADC conversion
	//while (!(ADCSRA & (1<<ADSC))); //wait for ADC conversion to complete (13 cycles)
	//adc_value = ADCL;
	//adc_value = adc_value | (ADCH <<8);
	//adc_previous = adc_value;
	//PORTH |= (1<<POTMUX_EN0); //set POTMUX_EN0
	
}

ISR (TIMER2_OVF_vect) { //main scanning interrupt handler
	
	display_DEC(tune_offset, digit[place]);
	//if (place == 0) { //if place is 0, start a new ADC conversion
		//select POTMUX input
	//	if (ISW4_SW_ON) { //16X oversampling
			
			//uint16_t adc_sum = 0;
			//for (int i = 0; i < 16; i++) {
				//DATA_BUS = 0b00000111; //select Y7 (VR2 POT)
				//PORTH &= ~(1<<POTMUX_EN0); //clear POTMUX_EN0 to select input Y7 on U2
				//ADCSRA |= (1<<ADSC); //start ADC conversion
				//while (!(ADCSRA & (1<<ADSC))); //wait for ADC conversion to complete (13 cycles)
				//
				//adc_value = ADCL;
				//adc_value = adc_value | (ADCH <<8);				 		
				//PORTH |= (1<<POTMUX_EN0); //set POTMUX_EN0
				//adc_sum += adc_value;
			//}				
			//adc_previous = adc_value;
			//adc_value = adc_sum>>2; //right shift by 2 to convert 14 bit sum to 12 bit result
	
				
	//	} else {
			//DATA_BUS = 0;
			for (int i = 0; i <=15; i++)
			{
				//int i = 7;	
				//DATA_BUS = i; //set pot mux address on databus
				//DELAYS ADDED HERE TO ALLOW SETTLING TIMES FOR POT MUTLIPLEXER INPUTS.
				//NEED TO HAVE ONE CHANNEL GROUNDED TO RESET MULTIPLEXER INPUT - NOT IF DELAY BETWEEN SETTING CHANNEL ADDRESS ON MUX AND 
				//READING POT IS SUFFICIENTLY LONG. 2us IS ENOUGH TO PRODUCE >1-2 mV OFFSET FROM FULL SWEEP OF PREVIOUS POT CHANNEL
				//DATA_BUS = 0; //set multiplexer to pot 0, kept at GND				

				//_delay_us(4); //2 us is minimum delay required to produce mux channel cross talk of <= 1 bit. Still need to see if this is affected by S&H			
				ADCSRA |= (1<<ADSC); //start ADC conversion
				while ((ADCSRA & (1<<ADSC))); //wait for ADC conversion to complete (13 cycles of ADC clock) - need to figure out what to do with this time - would interrupt be more efficient?
				//note that ADSC reads HIGH as long as conversion is in progress, goes LOW when conversion is complete
			
								
				//adc_previous = adc_value;
				adc_value = ADCL;
				adc_value = adc_value | (ADCH <<8);
				
				//set up mux for next ADC read
				POT_MUX &= ~(1<<POTMUX_EN0); //clear POTMUX_EN0 to select input on U2
				DATA_BUS = i; //set pot mux address on databus				
				//dac_channel[i] = adc_value << 4; //convert 10 bit ADC value to 14 bit DAC value
				//set_dac(i, dac_channel[i]); //set DAC
				//for testing, set one DAC S&H channel to a fixed value and measure it as flanking S&H channels are swept from 0-10V
				//currently using this to set OSCA_INIT_CV and do fine tuning
				if (i == 4 || i == 13) 
				{
					uint16_t tune_value = 6303;//9759; //init CV offset of about -5.8V
					if (adc_value >= 512) {set_dac(i,(tune_value + (adc_value - 512))); tune_offset = adc_value - 512;} else {set_dac(i,(tune_value - (512- adc_value))); tune_offset = adc_value;}
					//set_dac(i, tune_value);
					
				} else {set_dac(i, adc_value << 4);}
                //set_dac(i, adc_value << 4);
				POT_MUX |= (1<<POTMUX_EN0); //disable pot multiplexer U2
				//POT_MUX |= (1<<POTMUX_EN1); //needed to set this for some reason otherwise was reading both pot demuxers at once - need to check this out.	
						
			}			
			DAC_CTRL &= ~(1<<DAC_RS); //reset DAC
			DAC_CTRL |= (1<<DAC_RS);	
	//	}		
		//int deflection = adc_value - adc_previous;
		//if (deflection < 0 ) deflection = adc_previous - adc_value;
		//if (deflection <= 1) adc_value = adc_previous;
	//}				
	//toggle ARP_SYNC LED
	PINB = (1<<ARP_SYNC_LED);
	SPI_PORT |= SPI_SW_LATCH;
		
	//SHIFT 5th BYTE
	SPDR =  ISW4_SW_ON << 1 | ISW8_SW_ON << 7; //ISW8_LED is MSB on 74XX595 U16
	while (!(SPSR & (1<<SPIF)));
		
	//Now read SPDR for switch data shifted in from 74XX165 U14
	if (SPDR >> 7 & 1) //check if ISW4_SW bit is set (MSB on U14)
	{
		ISW4_SW_ON = 1;
	}
	else
	{
		ISW4_SW_ON = 0;
	}
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
	
	//check it see if ISW8_SW is ON
	if (SWITCH_PORT & (1<<ISW8_SW))
	{
		ISW8_SW_ON = 1;
	} else {
		ISW8_SW_ON = 0;
	}	
	//update 7-segment LED display 
	//int display_value;
	//if (ISW4_SW_ON) {
		//
			//display_value = (float(adc_value)/4092)*10000;
		//
		//} else {
		//
			//display_value = (float(adc_value)/1024)*10000; //at the moment, only last read POT (0b111) value is displayed
		//}			
	//display_DEC(display_value, digit[place]);

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
	
	//SET SPI_EN and LED_LATCH and VCO_SW_LATCH pins as outputs
	DDRJ |= (SPI_EN | LED_LATCH | (1<<VCO_SW_LATCH));
	
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
    setupADC();	
	
	//setup DAC
	setupDAC();
	
	//set up main timer interrupt
	//this generates the main scanning interrupt
	TCCR2A |= (1<<CS22) | (1<<CS21); //Timer2 20MHz/256 prescaler
	TIMSK2 |= (1<<TOIE2); //enable Timer2 overflow interrupt over flows approx. every 3ms	
	sei(); //enable global interrupts
	//POT_MUX |= (1<<POTMUX_EN1); //set pot mux en1 again - it is getting cleared somewhere	
	VCO_SW_LATCH_PORT &= ~(1<<VCO_SW_LATCH);
	//enable output on VCO analog switch latch:
	//switch latch: bit 0: A SAW 1: A PULSE 2: A TRI 3: SYNC 4: B MOD 5: B PULSE 6: SAW 7: B TRI
	DATA_BUS = 0b11010101; //enable VCO1 SAW
	VCO_SW_LATCH_PORT |= (1<<VCO_SW_LATCH);
	_delay_us(1);
	VCO_SW_LATCH_PORT &= ~(1<<VCO_SW_LATCH);
	DATA_BUS = 0;
	while(1)
	{
		
		//PORTB |= (1<<ARP_SYNC_LED);
		
		//SET SPI_SW_LATCH HI - this latches switch data into 74XX165 shift registers for SPI transfer

		
		
		
	}
}