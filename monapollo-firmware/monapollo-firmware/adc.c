#define F_CPU 20000000UL
#include <avr/io.h>
#include <util/delay.h>

#include "adc.h"
#include "hardware.h"

void setup_adc(void)
{
	//ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0); //set ADC clock to 156.25 KHz for 20 MHz clock
	//ADCSRA |= (1<<ADPS2) | (1<<ADPS1); //set ADC clock to 312.5 KHz for 20 MHz clock
	ADCSRA |= (1<<ADPS2);// | (1<<ADPS0); //set ADC clock to 1.25 MHz for 20 MHz clock
	ADMUX |= (1<<REFS0); //set ADC reference to AVCC (+5V)
	
	DIDR0 |= 0x01; //disable digital input buffer for ADC0
	
	ADCSRA |= (1<<ADEN); //enable ADC
}

uint16_t read_pot(uint8_t mux_select, uint8_t channel) {
	
	DATA_BUS = channel;
	POT_MUX &= ~(1<<mux_select);
	_delay_us(2); //ADC settling time. Previously used 10 us, testing 2 us now.
	ADCSRA |= (1<<ADSC); //start ADC conversion
	while ((ADCSRA & (1<<ADSC))); //wait for ADC conversion to complete (13 cycles of ADC clock - 10.4 us for 1.25Mhz clock) - need to figure out what to do with this time - would interrupt be more efficient?
	POT_MUX |= (1<<mux_select); //disable pot multiplexer U2
	//note that ADSC reads HIGH as long as conversion is in progress, goes LOW when conversion is complete
			
			
	uint16_t adc_read = ADCL;
	adc_read = adc_read | (ADCH <<8);
			
	return adc_read;
}