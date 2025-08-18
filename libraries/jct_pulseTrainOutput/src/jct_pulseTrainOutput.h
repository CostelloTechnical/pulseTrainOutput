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
#ifndef JCT_PULSETRAINOUTPUT_H
#define JCT_PULSETRAINOUTPUT_H
#include <Arduino.h>

enum pulseModes {
    STOP = 0,
    DISCRETE = 1,
    CONTINUOUS = 2
};

enum microcontrollers {
    UNKNOWN = 0,
    UNO = 1,  // Allowable pin: D9. Compatable with UNO, UNO Rev2 and UNO R3.
    NANO = 2, // Allowable pin: D9. Compatable with Nano (basic model only).
    MINI = 3, // Allowable pin: D9. 
    MEGA = 4  // Allowable pin: D11.
};

class pulseTrainOutput{
  public:
    // constructor
    pulseTrainOutput (microcontrollers microcontroller);
    static pulseTrainOutput* _classPointer;
    static volatile bool _pulseState;
    static volatile uint8_t _pin;
    static volatile uint8_t _pulseMode;
    static volatile uint32_t _pulseCounter;
    static volatile uint32_t _pulses;

    void generate (const uint32_t Hz, uint32_t pulses, pulseModes mode);
    void stop ();


  private:
    // addresses of output ports - NULL if not applicable
    volatile uint8_t * _timerRegA;   // Timer Control Register A
    volatile uint8_t * _timerRegB;   // Timer Control Register B
    volatile uint8_t * _timerOCRH;   // Output Compare Register High Byte
    volatile uint8_t * _timerOCRL;   // Output Compare Register Low Byte
    volatile uint8_t * _timerTCNTH;  // Timer/Counter High Byte
    volatile uint8_t * _timerTCNTL;  // Timer/Counter Low Byte
  };
#endif