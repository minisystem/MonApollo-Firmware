/*
 * Digital_Board_SPI_Test.cpp
 *
 * Created: 2015-03-17 3:01:46 PM
 *  Author: jeff
 *
 *SPI INTERFACE TEST WITH 74XX595 and 74XX165 SHIFT REGISTERS
 ==========================================================
 
 DONE:
 ==============================================================================
 
 Read ISW12 SW on MISO SPI bus from 74XX165 and light ISW12 LED if SW is ON.
 
 Test the entire 74XX595 SPI chain by turning ISW11 and ISW12 ON, ISW8 OFF on once cycle 
 and ISW11/12 OFF and ISW8 ON on the next cycle.
 
 ARP_SYNC (ISW13) is also being driven directly by the AVR
 
 TO DO:
 ==============================================================================
 
 Read another switch and illuminate ISW8.
 
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

#define ARP_SYNC_LED PB7

#define SPI_DATA_OUT	(1<<PB2)
#define SPI_DATA_IN		PB3
#define SPI_CLK			(1<<PB1)

#define SPI_EN			(1<<PJ2)

#define LED_LATCH		(1<<PJ3)

#define SPI_PORT PORTB
#define SPI_LATCH_PORT PORTJ

//define LED bits
#define ISW12_LED			0b00000100
#define ISW11_LED			0b10000000
#define ISW8_LED			0b10000000

//define switch bits
#define ISW12_SW			0b00100000

#define SPI_SW_LATCH		(1<<PB5)

int main(void)
{
	
	//SET PORTB PIN 7 (PB7) as OUTPUT
	DDRB |= (1<<ARP_SYNC_LED);
	
	//SET SPI_DATA_OUT and SPI_CLK pins as outputs
	//also set Slave Select (PB0) as output just to ensure it doesn't interfere with SPI communication (currently floating)
	//ACTUALLY, Slave Select ***MUST*** be set as output. Leaving it floating without setting its data direction bit breaks SPI!
	DDRB |= (SPI_DATA_OUT | SPI_CLK | SPI_SW_LATCH |(1<<PB0));
	
	//SET SPI_EN and LED_LATCH pins as outputs
	DDRJ |= (SPI_EN | LED_LATCH);
	
	//SET SPI_DATA_OUT and SPI_CLK and SPI_SW_LATCHoutputs LOW
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
	
	uint8_t ISW12_SW_ON = 0;
	
	while(1)
	{
		
		PORTB |= (1<<ARP_SYNC_LED);
		
		//SET SPI_SW_LATCH HI - this latches switch data into 74XX165 shift registers for SPI transfer
		SPI_PORT |= SPI_SW_LATCH;		
		
		//SHIFT 5th BYTE
		SPDR =  0; //ISW8_LED is MSB on 74XX595 U16
		while (!(SPSR & (1<<SPIF)));
		
		//SHIFT 4th BYTE
		SPDR = 0;
		while (!(SPSR & (1<<SPIF)));
		//check if ISW12_SW bit is set
		if (SPDR >> 5 & 1)
		{
			ISW12_SW_ON = 1;
		}
		else
		{
			ISW12_SW_ON = 0;
		}
		
		//SHIFT 3th BYTE
		SPDR = 0;
		while (!(SPSR & (1<<SPIF)));				

		//SHIFT 2th BYTE
		SPDR = 0;
		while (!(SPSR & (1<<SPIF)));
					
		//SHIFT 1st BYTE
		SPDR = (ISW12_SW_ON << 2) | ISW11_LED; //TURN ON ISW12 and ISW11 LEDs, both on 74XX595 U8, first shift register in chain
		//Wait for SPI shift to complete
		while (!(SPSR & (1<<SPIF)));
		
		//Toggle LED_LATCH to shift data to 74HC595 shift register outputs
		
		SPI_LATCH_PORT &= ~LED_LATCH;
		SPI_LATCH_PORT |= LED_LATCH;
		
		//clear SPI_SW_LATCH
		SPI_PORT &= ~SPI_SW_LATCH;
		
		/*
		_delay_ms(500);
		
		
		PORTB &= ~(1<<ARP_SYNC_LED);

		//SET SPI_SW_LATCH HI - this latches switch data into 74XX165 shift registers for SPI transfer
		SPI_PORT |= SPI_SW_LATCH;
		
		//SHIFT 5th BYTE
		SPDR = ISW8_LED; //turn on ISW8
		while (!(SPSR & (1<<SPIF)));
		
		//SHIFT 4th BYTE
		SPDR = 0;
		while (!(SPSR & (1<<SPIF)));		
		//check if ISW12_SW bit is set
		if (SPDR >> 5 & 1)
		{
			ISW12_SW_ON = 1;
		}
		else
		{
			ISW12_SW_ON = 0;
		}	
		
		//SHIFT 3th BYTE
		SPDR = 0;
		while (!(SPSR & (1<<SPIF)));
	
		//SHIFT 2th BYTE
		SPDR = 0;
		while (!(SPSR & (1<<SPIF)));		
		
		//SHIFT 1st BYTE
		SPDR = ISW12_SW_ON << 2; //turn off ISW12 and ISW11. Note that this just inverts the byte, but there are no other LEDs connected to this 595 during SPI testing
		//Wait for SPI shift to complete
		while (!(SPSR & (1<<SPIF)));
		
		//Toggle LED_LATCH to shift data to 74HC595 shift register outputs
		
		SPI_LATCH_PORT &= ~LED_LATCH;
		SPI_LATCH_PORT |= LED_LATCH;
		
		//SET SPI_SW_LATCH HIGH for asynchronous transfer
		SPI_PORT |= SPI_SW_LATCH;
				
		_delay_ms(500);
		*/
	}
}