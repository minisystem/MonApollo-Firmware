#ifndef HARDWARE_H
#define HARDWARE_H

//define PORTS
#define SPI_PORT			PORTB
#define SPI_LATCH_PORT		PORTJ
#define VCO_SW_LATCH_PORT	PORTJ
#define EG2_POL_PORT		PORTJ
#define DATA_BUS			PORTA
#define DISPLAY_PORT		PORTH
#define DAC_BUS_LOW			PORTD
#define DAC_BUS_HIGH		PORTC
#define DAC_CTRL			PORTG
#define DAC_MUX				PORTH
#define POT_MUX				PORTH

#define SWITCH_PORT			PINF //direct reading by MCU of some switches occurs on this port

//define hardware pins
#define TUNE_SELECT PG2 //define tune source select bit

#define GATE PF1 //define gate output

//switch latch bits for VCO waveforms, SYNC and BMOD
#define VCO1_SAW		0
#define VCO1_TRI		2
#define VCO1_PULSE		1
#define VCO2_SAW		6
#define VCO2_TRI		7
#define VCO2_PULSE		5
#define SYNC			3
#define BMOD			4

//define minimum and maximum values for 14 bit DAC
#define MIN 0
#define MAX 0x3FFF

//define pins for SPI
//SPI pins
#define SPI_DATA_OUT	(1<<PB2)
#define SPI_DATA_IN		PB3
#define SPI_CLK			(1<<PB1)

//SPI EN pin on PORTJ
#define SPI_EN			(1<<PJ2)

//LED latch pin on PORTJ
#define LED_LATCH		(1<<PJ3)

//SPI switch latch
#define SPI_SW_LATCH		(1<<PB5)

//define analog switch latch (VCO waveform switching)
#define VCO_SW_LATCH		PJ6
//define LFO waveform switch latch (not yet implemented in hardware)
#define LFO_SW_LATCH		PJ5

//EG2 polarity pin on PORTJ
#define EG2_POL			PJ4

#endif