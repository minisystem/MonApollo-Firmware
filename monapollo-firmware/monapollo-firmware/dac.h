#ifndef DAC_H
#define DAC_H


//define DAC bits
#define DAC_WR			PG0
#define DAC_RS			PG1
#define DAC_MUX_EN0		PH0 //DG408 enable is active HIGH
#define DAC_MUX_EN1		PH1
#define DAC_MUX_EN2		PH2
#define DAC_MUX_EN3		PH3

void setup_dac(void);

void set_dac(uint8_t dac_mux_address, uint8_t channel, uint16_t value);

#endif