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