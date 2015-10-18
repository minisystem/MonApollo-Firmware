//clock for Arpegiator and for LFO reset syncing
//source is either midi clock or arp clock

#include <avr/io.h>

#include "clock.h"

struct system_clock clock;