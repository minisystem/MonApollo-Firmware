#ifndef SPI_H
#define SPI_H

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

void update_spi(void);

#endif