#ifndef ADC_H
#define ADC_H

//define POTMUX_EN bits
#define POTMUX_EN0	PH6
#define POTMUX_EN1	PH7

void setup_adc(void);

uint16_t read_pot(uint8_t mux_select, uint8_t channel);

#endif
