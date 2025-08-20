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

// An enum to help us index our static array

enum timerIds {
    TID_TIMER1,
    TID_TIMER2,
    TID_TIMER3,
    TID_TIMER4,
    TID_TIMER5,
    TID_INVALID
};

class pulseTrainOutput{
  public:
    // --- Static Array for ISRs ---
    // An array of pointers, one for each possible timer instance.
    static pulseTrainOutput* _instances[5];

    // Constructor: Takes a pin and automatically configures for the correct timer.
    pulseTrainOutput(uint8_t pin);

    // Start generating pulses. Returns false if frequency is out of range.
    bool generate(uint32_t frequency, pulseModes mode = CONTINUOUS, uint32_t pulses = 1);
    
    // Stop generating pulses. 
    void stop();

    // Check if the pulse train is currently active.
    bool isRunning() const;
    bool isValid() const;

    // Public interrupt handler that the global ISRs will call.
    void handleInterrupt();

private:
    // --- Member Variables for this instance ---
    uint8_t _pin;
    timerIds _timerId; // Which hardware timer this object controls
    bool _is16bit;    // Flag to handle 8-bit vs 16-bit timer differences

    // Pointers to the hardware registers for this timer.
    volatile uint8_t* _tccrA;
    volatile uint8_t* _tccrB;
    volatile uint8_t* _timsk;
    volatile uint16_t* _ocr; // Use 16-bit pointer for both for simplicity
    volatile uint8_t* _outputPort;

    // Interrupt control
    uint8_t _ocieBit; // The bit to set in the TIMSK register
    uint8_t _comStopMask;
    uint8_t _comToggleBit;

    // State variables
    volatile uint32_t _pulseCounter;
    volatile bool _pulseState;
    volatile uint32_t _pulsesToGenerate;
    volatile uint8_t _pulseMode;
    volatile bool _isRunning;
    uint8_t _pinBitMask;
};

#endif // JCT_PULSETRAINOUTPUT_H