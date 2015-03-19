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
 
 Test the entire 74XX595 SPI chain by turning ISW11 and ISW12 ON, ISW8 OFF on once cycle 
 and ISW11/12 OFF and ISW8 ON on the next cycle.
 
 ARP_SYNC (ISW13) is also being driven directly by the AVR
 
 TO DO:
 ==============================================================================
 
 Implement SPI switch reading using 74XX165 parallel to serial shift registers
 
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

#define ISW12_LED			0b00000100
#define ISW11_LED			0b10000000
#define ISW8_LED			0b10000000

int main(void)
{
	
	//SET PORTB PIN 7 (PB7) as OUTPUT
	DDRB |= (1<<ARP_SYNC_LED);
	
	//SET SPI_DATA_OUT and SPI_CLK pins as outputs
	//also set Slave Select (PB0) as output just to ensure it doesn't interfere with SPI communication (currently floating)
	//ACTUALLY, Slave Select ***MUST*** be set as output. Leaving it floating without setting its data direction bit breaks SPI!
	DDRB |= (SPI_DATA_OUT | SPI_CLK | (1<<PB0));
	
	//SET SPI_EN and LED_LATCH pins as outputs
	DDRJ |= (SPI_EN | LED_LATCH);
	
	//SET SPI_DATA_OUT and SPI_CLK outputs LOW
	SPI_PORT &= ~(SPI_DATA_OUT | SPI_CLK);
	
	//SET SPI_EN LOW (active) and LED_LATCH LOW (active)
	SPI_LATCH_PORT &= ~(SPI_EN | LED_LATCH);
	
	//SET UP SPI
	SPCR = (1<<SPE) | (1<<MSTR); //Start SPI as MASTER
	
	//Pull LED_LATCH LOW
	SPI_LATCH_PORT &= ~LED_LATCH;
	
	//SHIFT DATA IN TO LIGHT ISW12
	SPDR = 0b11111111;
	//Wait for SPI shift to complete
	while (!(SPSR & (1<<SPIF)));
	
	//Toggle LED_LATCH to shift data to 74HC595 shift register outputs
	
	SPI_LATCH_PORT &= ~LED_LATCH;
	SPI_LATCH_PORT |= LED_LATCH;
	
	while(1)
	{
		
		PORTB |= (1<<ARP_SYNC_LED);
		
		//SHIFT 5th BYTE
		SPDR = 0; //ISW8_LED is MSB on 74XX595 U16
		while (!(SPSR & (1<<SPIF)));
		
		//SHIFT 4th BYTE
		SPDR = 0;
		while (!(SPSR & (1<<SPIF)));

		//SHIFT 3th BYTE
		SPDR = 0;
		while (!(SPSR & (1<<SPIF)));				

		//SHIFT 2th BYTE
		SPDR = 0;
		while (!(SPSR & (1<<SPIF)));
				
		//SHIFT 1st BYTE
		SPDR = ISW12_LED | ISW11_LED; //TURN ON ISW12 and ISW11 LEDs, both on 74XX595 U8, first shift register in chain
		//Wait for SPI shift to complete
		while (!(SPSR & (1<<SPIF)));
		
		//Toggle LED_LATCH to shift data to 74HC595 shift register outputs
		
		SPI_LATCH_PORT &= ~LED_LATCH;
		SPI_LATCH_PORT |= LED_LATCH;
		
		_delay_ms(500);
		PORTB &= ~(1<<ARP_SYNC_LED);
		
		//SHIFT 5th BYTE
		SPDR = ISW8_LED; //turn on ISW8
		while (!(SPSR & (1<<SPIF)));
		
		//SHIFT 4th BYTE
		SPDR = 0;
		while (!(SPSR & (1<<SPIF)));

		//SHIFT 3th BYTE
		SPDR = 0;
		while (!(SPSR & (1<<SPIF)));

		//SHIFT 2th BYTE
		SPDR = 0;
		while (!(SPSR & (1<<SPIF)));
		
		//SHIFT 1st BYTE
		SPDR = ~(ISW12_LED | ISW11_LED); //turn off ISW12 and ISW11. Note that this just inverts the byte, but there are no other LEDs connected to this 595 during SPI testing
		//Wait for SPI shift to complete
		while (!(SPSR & (1<<SPIF)));
		
		//Toggle LED_LATCH to shift data to 74HC595 shift register outputs
		
		SPI_LATCH_PORT &= ~LED_LATCH;
		SPI_LATCH_PORT |= LED_LATCH;
		_delay_ms(500);
		
	}
}