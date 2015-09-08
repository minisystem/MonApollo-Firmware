#include <avr/io.h>
#include <util/delay.h>

#include "spi.h"
#include "switch_map.h"
#include "led_map.h"
#include "hardware.h"
#include "tune.h"
#include "utils.h"

//switch flags
//ultimately combine these into a single byte and do bit manipulations to determine switch states
static uint8_t EG2_INV_ON = 0; //flag for ISW9 switch
static uint8_t PROG_WRITE_ON = 0; //flag for ISW11 switch
static uint8_t ARP_MODE_SW_ON = 0; //flag for ISW12 switch
static uint8_t ARP_SYNC_SW_ON = 0; //flag for ISW13 switch
static uint8_t VCO_SYNC_SW_ON = 0;  //flag for ISW4 switch
static uint8_t BMOD_SW_ON = 0; //flag for ISW8 switch (direct bus into MCU)

//debounce spi switch state flags
static uint8_t spi_sw_byte0_current_state = 0;  //U14 switch latch 5th byte in SPI chain, first byte read
static uint8_t spi_sw_byte0_previous_state = 0; 
static uint8_t spi_sw_byte1_current_state = 0;  //U9  switch latch 4th byte in SPI chain, second byte read
static uint8_t spi_sw_byte1_previous_state = 0;
//switch state holder - these hold the toggled states that the LED states and modes are derived from
//static uint8_t switch_state_byte0 = 0; //U14 switch latch 5th byte in SPI chain, first byte read
//static uint8_t switch_state_byte1 = 0; //U9  switch latch 4th byte in SPI chain, second byte read


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
			uint8_t spi_data = //Need to farm this out to some kind of switch/LED data parser that preps LED SPI outgoing bytes
			((switch_states.byte0 >> VCO_SYNC_SW) & 1) << VCO_SYNC |
			((switch_states.byte0 >> VCO1_SAW_SW) & 1) << VCO1_SAW |
			((switch_states.byte0 >> VCO1_TRI_SW) & 1) << VCO1_TRI |
			((switch_states.byte0 >> VCO1_PULSE_SW) & 1) << VCO1_PULSE |
			((switch_states.byte0 >> VCO2_SAW_SW) & 1) << VCO2_SAW |
			((switch_states.byte0 >> VCO2_TRI_SW) & 1) << VCO2_TRI |
			((switch_states.byte0 >> VCO2_PULSE_SW) & 1) << VCO2_PULSE |
			BMOD_SW_ON << BMOD;
			
					
			//Now read SPDR for switch data shifted in from 74XX165 U14
			spi_sw_byte0_current_state = spi_shift_byte(spi_data);
			
			spi_sw_byte0_current_state ^= spi_sw_byte0_previous_state;
			spi_sw_byte0_previous_state ^= spi_sw_byte0_current_state;
			spi_sw_byte0_current_state &= spi_sw_byte0_previous_state;
			
			//toggle switch state		
			switch_states.byte0 ^= spi_sw_byte0_current_state; //Omar's solution.
			
			//SHIFT 4th BYTE
			//Now read SPDR for switch data shifted in from 74XX165 (U9)
			spi_data = (1<<VCO2_32F | 1<<VCO1_32F); //turn on 32' octave LEDs as default 
			//switch_state_byte1 = spi_shift_byte(spi_data);
			
			spi_sw_byte1_current_state = spi_shift_byte(spi_data);
	
			spi_sw_byte1_current_state ^= spi_sw_byte1_previous_state;
			spi_sw_byte1_previous_state ^= spi_sw_byte1_current_state;
			spi_sw_byte1_current_state &= spi_sw_byte1_previous_state;
			
			//toggle switch state
			switch_states.byte1 ^= spi_sw_byte1_current_state; //Omar's solution.			
			
			
			//check if ARP_MODE_SW bit is set
			ARP_MODE_SW_ON = (switch_states.byte1 >> ARP_MODE_SW) & 1;
			//check if ARP_SYNC_SW bit is set
			ARP_SYNC_SW_ON = (switch_states.byte1 >> ARP_SYNC_SW) & 1; //not currently used

			
			//SHIFT 3th BYTE
			spi_shift_byte(0);

			//SHIFT 2th BYTE
			spi_shift_byte(0);
			
			//SHIFT 1st BYTE			
			spi_data = (ARP_MODE_SW_ON << ARP_MODE) | (PROG_WRITE_ON << PROG_WRITE) | (EG2_INV_ON << EG2_INV); 
			//Wait for SPI shift to complete
			spi_shift_byte(spi_data);
			
			//Toggle LED_LATCH to shift data to 74HC595 shift register outputs
			
			SPI_LATCH_PORT &= ~LED_LATCH;
			SPI_LATCH_PORT |= LED_LATCH;
			
			//clear SPI_SW_LATCH
			SPI_PORT &= ~SPI_SW_LATCH;
			
			//EVERYTHING BELOW NEEDS TO BE MOVED OUT OF SPI FUNCTION//
			
			//now read switches directly connected to MCU
			uint8_t current_sw_state = read_switch_port();
			
			if (current_sw_state & (1<<BMOD_SW))
			{
				BMOD_SW_ON ^= 1 << 0; //toggle switch state
			}
			
			if (current_sw_state & (1<<PROG_WRITE_SW)) {
				
				PROG_WRITE_ON ^= 1 << 0; //toggle switch state
			}
			
			if (current_sw_state & (1<<EG2_INV_SW)) {
				
				EG2_INV_ON ^= 1 << 0; //toggle switch state
			}
			
			//update analog switch latch:
			//need to incorporate BMOD_LATCH_BIT switch state into data byte sent to analog switch latch
			//3rd switch bit is VCO1_OCTAVE_UP_SW state, which isn't used by analog switch latch
			uint8_t switch_state_byte = switch_states.byte0;
			switch_state_byte ^= (-BMOD_SW_ON ^ switch_state_byte) & (1<<3);//set third bit dependent on 
			update_analog_switch_latch(switch_state_byte);

			
			//set EG2 INV bit. This changes the nth bit to x from: http://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit-in-c-c
			//need to make sure this doesn't interfere with anything else on this port
			EG2_POL_PORT ^= (-EG2_INV_ON ^ EG2_POL_PORT) & (1<<EG2_POL);
			
			if (PROG_WRITE_ON) { //temporary tune button hack
				
				PROG_WRITE_ON ^= 1<<0; //toggle switch state
				current_sw_state ^= (1<<PROG_WRITE_SW); //toggle read switch state
				//vco1_init_cv = set_vco_init_cv(VCO1, 24079);
				//vco2_init_cv = set_vco_init_cv(VCO2, 24079);
				tune_8ths(VCO1); 
				tune_8ths(VCO2);
				
				
			}
	
}