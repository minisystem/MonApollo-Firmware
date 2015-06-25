#include <avr/io.h>
#include <util/delay.h>

#include "spi.h"
#include "switch_map.h"
#include "led_map.h"
#include "port_map.h"

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

void update_spi(void) {
	
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

			//if (spi_sw_current_state & (1<<ISW1_SW)) sw_latch_five ^= (1 << ISW1_SW);
			//if (spi_sw_current_state & (1<<ISW2_SW)) sw_latch_five ^= (1 << ISW2_SW);
			//if (spi_sw_current_state & (1<<ISW3_SW)) sw_latch_five ^= (1 << ISW3_SW);
			//if (spi_sw_current_state & (1<<ISW4_SW)) sw_latch_five ^= (1 << ISW4_SW);
			//if (spi_sw_current_state & (1<<ISW5_SW)) sw_latch_five ^= (1 << ISW5_SW);
			//if (spi_sw_current_state & (1<<ISW6_SW)) sw_latch_five ^= (1 << ISW6_SW);
			//if (spi_sw_current_state & (1<<ISW7_SW)) sw_latch_five ^= (1 << ISW7_SW);
			
			sw_latch_five ^= spi_sw_current_state;
			
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