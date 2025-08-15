/*
  ==============================================================================
                                  DISCLAIMER
  ==============================================================================

  This software is provided "as is", without warranty of any kind, express or
  implied, including but not to the warranties of merchantability,
  fitness for a particular purpose and noninfringement. In no event shall the
  authors or copyright holders be liable for any claim, damages or other
  liability, whether in an action of contract, tort or otherwise, arising from,
  out of or in connection with the software or the use or other dealings in the
  software.

  ==============================================================================
                              PERMISSION TO USE
  ==============================================================================

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so.

  It is highly encouraged that if you find this library useful, you provide
  attribution back to the original author.
*/
#include "jct_pulseTrainOutput.h"


PCICR |= B00000001;  // Enables pin change interrupts on ports B and D. 
PCMSK0 |= B00000010; // Enables pin change interrupt on port B, Bit 1. Pin 9 on the nano/uno.


// Handles the pin change interrupt for the button on pin 9.
ISR(PCINT0_vect) { // Pins D8-D13
    uint8_t portB = PINB;
}