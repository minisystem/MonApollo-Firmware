#include <avr/io.h>
#include <util/delay.h>

#include "spi.h"
#include "switch_map.h"
#include "led_map.h"
#include "hardware.h"
#include "tune.h"

//switch flags
//ultimately combine these into a single byte and do bit manipulations to determine switch states
volatile uint8_t EG2_INV_ON = 0; //flag for ISW9 switch
volatile uint8_t PROG_WRITE_ON = 0; //flag for ISW11 switch
volatile uint8_t ISW12_SW_ON = 0; //flag for ISW12 switch
volatile uint8_t ISW13_SW_ON = 0; //flag for ISW13 switch
volatile uint8_t ISW4_SW_ON = 0;  //flag for ISW4 switch
volatile uint8_t BMOD_SW_ON = 0; //flag for ISW8 switch (direct bus into MCU)

//debounce switch state flags
//again, combine into a single byte and do bit manipulations to determine previous switch states
//flags for direct (into MCU) input switches
static uint8_t previous_sw_state = 0;
static uint8_t current_sw_state = 0;
//flags for SPI input switches (currently only 1 SPDR byte - will need to make these variables wider or have multiple flag bytes for each successive SPI byte)
volatile uint8_t spi_sw_current_state = 0;
volatile uint8_t spi_sw_previous_state = 0;
//switch state holder
volatile uint8_t sw_latch_five = 0; //U16 5th switch latch in SPI chain
volatile uint8_t sw_latch_four = 0;
volatile uint8_t sw_latch_three = 0;
volatile uint8_t sw_latch_two = 0;
volatile uint8_t sw_latch_one = 0;

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
			uint8_t spi_data = 
			((sw_latch_five >> ISW4_SW) & 1) << ISW4_LED |
			((sw_latch_five >> ISW1_SW) & 1) << ISW1_LED |
			((sw_latch_five >> ISW2_SW) & 1) << ISW2_LED |
			((sw_latch_five >> ISW3_SW) & 1) << ISW3_LED |
			((sw_latch_five >> ISW5_SW) & 1) << ISW5_LED |
			((sw_latch_five >> ISW6_SW) & 1) << ISW6_LED |
			((sw_latch_five >> ISW7_SW) & 1) << ISW7_LED |
			BMOD_SW_ON << ISW8_LED;
			
					
			//Now read SPDR for switch data shifted in from 74XX165 U14
			spi_sw_current_state = spi_shift_byte(spi_data);
			
			spi_sw_current_state ^= spi_sw_previous_state;
			spi_sw_previous_state ^= spi_sw_current_state;
			spi_sw_current_state &= spi_sw_previous_state;
			
			//toggle switch state		
			sw_latch_five ^= spi_sw_current_state; //Omar's solution.
			
			//SHIFT 4th BYTE
			//Now read SPDR for switch data shifted in from 74XX165 (U9)
			spi_data = (1<<VCO2_32F | 1<<VCO1_32F);
			sw_latch_four = spi_shift_byte(spi_data);
			//toggling not implemented here yet.
			ISW12_SW_ON = (sw_latch_four >> ISW12_SW) & 1;
			//check if ISW13_SW bit is set
			ISW13_SW_ON = (sw_latch_four >> ISW13_SW) & 1;

			
			//SHIFT 3th BYTE
			sw_latch_three = spi_shift_byte(0);

			//SHIFT 2th BYTE
			sw_latch_two = spi_shift_byte(0);
			
			//SHIFT 1st BYTE
			
			spi_data = (ISW12_SW_ON << ISW12_LED) | (PROG_WRITE_ON << ISW11_LED) | (EG2_INV_ON << ISW9_LED); 
			//Wait for SPI shift to complete
			sw_latch_one = spi_shift_byte(spi_data);
			
			//Toggle LED_LATCH to shift data to 74HC595 shift register outputs
			
			SPI_LATCH_PORT &= ~LED_LATCH;
			SPI_LATCH_PORT |= LED_LATCH;
			
			//clear SPI_SW_LATCH
			SPI_PORT &= ~SPI_SW_LATCH;
			
			//now read switches directly connected to MCU
			//this toggle code works, but I haven't figured out how it works
			//source: http://forum.allaboutcircuits.com/threads/help-with-programming-uc-toggle-led-using-one-switch.51602/
			current_sw_state = SWITCH_PORT;
			current_sw_state ^= previous_sw_state;
			previous_sw_state ^= current_sw_state;
			current_sw_state &= previous_sw_state;
			
			if (current_sw_state & (1<<BMOD_SW))
			{
				BMOD_SW_ON ^= 1 << 0; //toggle switch state
			}
			
			if (current_sw_state & (1<<PROG_WRITE)) {
				
				PROG_WRITE_ON ^= 1 << 0; //toggle switch state
			}
			
			if (current_sw_state & (1<<EG2_INV)) {
				
				EG2_INV_ON ^= 1 << 0; //toggle switch state
			}
			
			//update analog switch latch:
			VCO_SW_LATCH_PORT &= ~(1<<VCO_SW_LATCH);
			//enable output on VCO analog switch latch:
			//switch latch: 7: B TRI 6: B SAW 5: B PULSE 4: B MOD 3: SYNC 2: A TRI 1: A PULSE 0: A SAW
			DATA_BUS =
			((sw_latch_five >> ISW4_SW) & 1) << SYNC |
			((sw_latch_five >> ISW1_SW) & 1) << VCO1_SAW |
			((sw_latch_five >> ISW2_SW) & 1) << VCO1_TRI |
			((sw_latch_five >> ISW3_SW) & 1) << VCO1_PULSE |
			((sw_latch_five >> ISW5_SW) & 1) << VCO2_SAW |
			((sw_latch_five >> ISW6_SW) & 1) << VCO2_TRI |
			((sw_latch_five >> ISW7_SW) & 1) << VCO2_PULSE |
			BMOD_SW_ON << BMOD;
			VCO_SW_LATCH_PORT |= (1<<VCO_SW_LATCH);
			_delay_us(1); //why is this delay here????
			VCO_SW_LATCH_PORT &= ~(1<<VCO_SW_LATCH);
			DATA_BUS = 0;
			
			//set EG2 INV bit. This changes the nth bit to x from: http://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit-in-c-c
			//need to make sure this doesn't interfere with anything else on this port
			EG2_POL_PORT ^= (-EG2_INV_ON ^ EG2_POL_PORT) & (1<<EG2_POL);
			
			if (PROG_WRITE_ON) { //temporary tune button hack
				
				PROG_WRITE_ON ^= 1<<0; //toggle switch state
				current_sw_state ^= (1<<PROG_WRITE); //toggle read switch state
				//update_spi();
				//vco1_init_cv = set_vco_init_cv(VCO1, 24079);
				//vco2_init_cv = set_vco_init_cv(VCO2, 24079);
				tune_8ths(VCO1);
				tune_8ths(VCO2);
				
			}
	
}