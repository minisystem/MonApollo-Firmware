#define F_CPU 20000000UL

#include <avr/io.h>
#include <util/delay.h>

#include "hardware.h"
#include "dac.h"

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

	_delay_us(2); //AD5556 DAC has 0.5 us settling time. 1 us wasn't long enough for transitions from 10V to 0V
	DAC_MUX |= (1<<dac_mux_address); //enable multiplexer
	_delay_us(10); //wait for S&H cap to charge - need to figure out how to do this more time efficiently
	DAC_MUX &= ~(1<<dac_mux_address); //disable multiplexer
	
}

