#include <avr/io.h>
#include <util/delay.h>

#include "spi.h"
#include "switch_map.h"
#include "led_map.h"
#include "hardware.h"
#include "synth.h"
#include "tune.h"
#include "utils.h"

//debounce spi switch state bytes
static uint8_t spi_sw_byte0_current_state = 0;  //U14 switch latch 5th byte in SPI chain, first byte read
static uint8_t spi_sw_byte0_previous_state = 0; 
static uint8_t spi_sw_byte1_current_state = 0;  //U9  switch latch 4th byte in SPI chain, second byte read
static uint8_t spi_sw_byte1_previous_state = 0;

void setup_spi(void) {
	
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
		
		
		//Toggle LED_LATCH to shift data to 74HC595 shift register outputs
		
		SPI_LATCH_PORT &= ~LED_LATCH;
		SPI_LATCH_PORT |= LED_LATCH;
		
		//set EG2 POL
		EG2_POL_PORT &= ~(1 << EG2_POL); //0 for normal, 1 for inverted
	
}

uint8_t spi_shift_byte(uint8_t byte) { //shifts out byte for LED data and simultaneously reads switch data
	
	SPDR = byte;
	while (!(SPSR & (1<<SPIF)));
	return SPDR;
	
}

void update_spi(void) {
	
			SPI_PORT |= SPI_SW_LATCH;
			
			//SHIFT 5th BYTE				
			//Read SPDR for switch data shifted in from 74XX165 U14 and write LED data to LED latch 5
			spi_sw_byte0_current_state = spi_shift_byte(current_patch.byte_5);
			
			spi_sw_byte0_current_state ^= spi_sw_byte0_previous_state;
			spi_sw_byte0_previous_state ^= spi_sw_byte0_current_state;
			spi_sw_byte0_current_state &= spi_sw_byte0_previous_state;
			
			//toggle switch state		
			switch_states.byte0 ^= spi_sw_byte0_current_state; //Omar's solution.
			
			//SHIFT 4th BYTE
			//Now read SPDR for switch data shifted in from 74XX165 (U9)
			//uint8_t spi_data = (1<<VCO2_32F | 1<<VCO1_32F); //turn on 32' octave LEDs as default 
			
			uint8_t spi_data = current_patch.byte_4;
					
			spi_sw_byte1_current_state = spi_shift_byte(spi_data);
	
			spi_sw_byte1_current_state ^= spi_sw_byte1_previous_state;
			spi_sw_byte1_previous_state ^= spi_sw_byte1_current_state;
			spi_sw_byte1_current_state &= spi_sw_byte1_previous_state;
			
			//toggle switch state
			switch_states.byte1 ^= spi_sw_byte1_current_state; //Omar's solution.			
							
			//SHIFT 3th BYTE
			spi_shift_byte(current_patch.byte_3);

			//SHIFT 2th BYTE
			spi_shift_byte(current_patch.byte_2);
			
			//SHIFT 1st BYTE	//eventually need to parse this elsewhere		
			spi_data =	((switch_states.byte1 >> ARP_MODE_SW) & 1) << ARP_MODE | 
						((switch_states.byte2 >> PROG_WRITE_SW) & 1) << PROG_WRITE | 
						((switch_states.byte2 >> EG2_INV_SW) &1 ) << EG2_INV |
						((switch_states.byte2 >> PROG_MANUAL_SW &1) << PROG_MANUAL); 			
			//Wait for SPI shift to complete
			spi_shift_byte(spi_data);
			
			//Toggle LED_LATCH to shift data to 74HC595 shift register outputs
			
			SPI_LATCH_PORT &= ~LED_LATCH;
			SPI_LATCH_PORT |= LED_LATCH;
			
			//clear SPI_SW_LATCH
			SPI_PORT &= ~SPI_SW_LATCH;
			
			

	
}