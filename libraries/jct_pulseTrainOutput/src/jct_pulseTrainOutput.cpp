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
pulseTrainOutput* pulseTrainOutput::_classPointer = nullptr;
volatile bool pulseTrainOutput::_pulseState = false;
volatile uint8_t pulseTrainOutput::_pulseMode = STOP;
volatile uint32_t pulseTrainOutput::_pulseCounter = 0;
volatile uint32_t pulseTrainOutput::_pulses = 0;

// Handles the pin change interrupt for the button on pin 9.
ISR(PCINT0_vect) { // Pins D8-D13
  uint8_t portB = PINB;
  if (pulseTrainOutput::_pulseMode == DISCRETE){
    pulseTrainOutput::_pulseCounter += (portB & B00000010)==B00000010;
    if (pulseTrainOutput::_pulseCounter >= pulseTrainOutput::_pulses){
      if ((portB & B00000010)==B00000000){
        pulseTrainOutput::_classPointer->stop();
      }
    }
  }
}

pulseTrainOutput::pulseTrainOutput (microcontrollers microcontroller, pins pin){
  _classPointer = this;
  if(microcontroller == UNO | microcontroller == NANO){
    if(pin == D9){
      PCICR |= B00000001;  // Enables pin change interrupts on ports B. 
      PCMSK0 |= B00000010; // Enables pin change interrupt on port B, Bit 1. Pin 9 on the nano/uno.
      pinMode (D9, OUTPUT);
      _timerRegA  = TCCR1A;
      _timerRegB  = TCCR1B;
      _timerOCRH  = OCR1AH;
      _timerOCRL  = OCR1AL;
      _timerTCNTH = TCNT1H;
      _timerTCNTL = TCNT1L;
    }
  }//
  else if(microcontroller == MEGA){
    
  }
}

void pulseTrainOutput::generate (const uint32_t Hz, uint32_t pulses, pulseModes mode)
{
  // it takes two toggles for one "cycle"
  unsigned long ocr = F_CPU / Hz / 2;
  uint8_t prescaler = _BV (CS10);  // start with prescaler of 1  (bits are the same for all timers)
  _pulses = pulses;
  _pulseMode = mode;
  // too large? prescale it
  if (ocr > 0xFFFF)
    {
    prescaler |= _BV (CS11);    // now prescaler of 64
    ocr /= 64;
    }
  
  // stop timer
  *_timerRegA = 0;
  *_timerRegB = 0;
  
  // reset counter
  *_timerTCNTH = 0;
  *_timerTCNTL = 0;
  
  // what to count up to
  *_timerOCRH = highByte (ocr);
  *_timerOCRL = lowByte (ocr);
  
  *_timerRegA = _BV (COM1A0);             // toggle output pin
  *_timerRegB = _BV (WGM12) | prescaler;  // CTC
  }  // end of TonePlayer::tone

void pulseTrainOutput::stop ()
  {
  // stop timer
  *_timerRegA = 0;
  *_timerRegB = 0;
  _pulseMode = STOP;
  _pulseCounter = 0;
  } // end of TonePlayer::noTone