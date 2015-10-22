#ifndef HARDWARE_H
#define HARDWARE_H

//define PORTS
#define SPI_PORT			PORTB
#define SPI_LATCH_PORT		PORTJ
#define LFO_LATCH_PORT		PORTJ
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

//switch latch bits for VCO waveforms, VCO_SYNC_LATCH_BIT and BMOD_LATCH_BIT
#define VCO1_SAW_LATCH_BIT		0
#define VCO1_TRI_LATCH_BIT		2
#define VCO1_PULSE_LATCH_BIT	1
#define VCO2_SAW_LATCH_BIT		6
#define VCO2_TRI_LATCH_BIT		7
#define VCO2_PULSE_LATCH_BIT	5
#define VCO_SYNC_LATCH_BIT		3
#define BMOD_LATCH_BIT			4

//define minimum and maximum values for 14 bit DAC
#define MIN 0
#define MAX 0x3FFF

//define boolean states for code readibility
#define TRUE 1
#define FALSE 0

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



//define LFO DEMUX waveform selection bytes:
#define LFO_TRI_ADDR	0
#define LFO_SINE_ADDR	4
#define LFO_SAW_ADDR	2
#define LFO_PULSE_ADDR	6
#define LFO_RNDM_ADDR	1	

#define LFO_RESET		PB4 //define LFO reset pin

//EG2 polarity pin on PORTJ
#define EG2_POL			PJ4

//LED defines
//define LED bits - number corresponds to bit position in serial-parallel shift register (74HC595)
//the order of these bits are also used to store and recall and set patch data so all analog switch settings and sync/arp modes are derived from these bit mappings

#define BMOD				7 //B MOD
#define VCO2_PULSE			6 //VCO2 PULSE U16
#define VCO2_TRI			5 //VCO2 TRI U16
#define VCO2_SAW			4 //VCO2 SAW U16
#define VCO1_TRI			3 //VCO1 TRI U16
#define VCO1_SAW			2 //VCO1 SAW U16
#define VCO_SYNC			1 //VCO_SYNC_LATCH_BIT U16
#define VCO1_PULSE			0 //VCO1 PULSE U16

#define VCO2_2F				0 //LED12 on LED LATCH 3
#define VCO2_4F				1 //LED11 on LED LATCH 3
#define VCO2_8F				7 //LED10 on LED LATCH 4
#define VCO2_16F			6 //LED9 on LED LATCH 4
#define VCO2_32F			5 //LED8 on LED LATCH 4

#define VCO1_2F				4 //LED7 on LED LATCH 4
#define VCO1_4F				3 //LED6 on LED LATCH 4
#define VCO1_8F				0 //LED5 on LED LATCH 4
#define VCO1_16F			2 //LED4 on LED LATCH 4
#define VCO1_32F			1 //LED3 on LED LATCH 4

#define PROG_WRITE			7 //WRITE
#define PROG_MANUAL			6 //MANUAL
#define ARP_MODE			2 //ARP MODE
#define EG2_INV				0 //EG2 INV

#define LFO_TRI				7 //LED17 on LED LATCH 2
#define LFO_SINE			6 //LED18 on LED LATCH 2
#define LFO_SAW				5 //LED19 on LED LATCH 2
#define LFO_RNDM			4 //LED20 on LED LATCH 2
#define LFO_PULSE			0b00110000 //STUPIDHEAD DID NOT MAKE A LED FOR LFO PULSE. LIGHT UP SAW AND RNDMN INSTEAD


#define LFO_KEY_SYNC 0 //SYNC LED for LFO note-on reset. Didn't really think about this until after hardware was designed. This brings display of other sync timings from 4 down to 3
#define LFO_SYNC_1   1 //these names are kind of arbitrary as the sync divide is set by clock.divide parameter
#define LFO_SYNC_2   2
#define LFO_SYNC_4   3

//ARP LED constants all in patch.byte_3 except ARP_RANGE_1 which is in patch.byte_1
#define ARP_RANGE_3	7
#define ARP_RANGE_2 6
#define ARP_SYNC_2	5
#define ARP_SYNC_4	4
#define ARP_SYNC_8	3
#define ARP_SYNC_16	2

//other ARP LEDs in patch.byte_1
#define ARP_MODE_UP	5
#define ARP_MODE_DN	4
#define ARP_MODE_RD	3
#define ARP_ON		2
#define ARP_RANGE_1 1

#define TIMER1_DIVIDE_64	(1<<CS11) | (1<<CS10)
#define TIMER1_DIVIDE_1024	(1<<CS12) | (1<<CS10)

//ARP_SYNC LED driven directly from AVR
#define ARP_SYNC_LED	PB7

#endif