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

class jctPulseTrainOutput
  {
  // addresses of output ports - NULL if not applicable
  volatile byte * const _timerRegA;   // Timer Control Register A
  volatile byte * const _timerRegB;   // Timer Control Register B
  volatile byte * const _timerOCRH;   // Output Compare Register High Byte
  volatile byte * const _timerOCRL;   // Output Compare Register Low Byte
  volatile byte * const _timerTCNTH;  // Timer/Counter High Byte
  volatile byte * const _timerTCNTL;  // Timer/Counter Low Byte
  
  public:
    // constructor
    TonePlayer (
          // ports
          volatile byte & timerRegA, 
          volatile byte & timerRegB, 
          volatile byte & timerOCRH,
          volatile byte & timerOCRL, 
          volatile byte & timerTCNTH, 
          volatile byte & timerTCNTL)
       : 
         _timerRegA  (&timerRegA), 
         _timerRegB  (&timerRegB),
         _timerOCRH  (&timerOCRH), 
         _timerOCRL  (&timerOCRL), 
         _timerTCNTH (&timerTCNTH), 
         _timerTCNTL (&timerTCNTL)
  { }
    
    void  (const unsigned int Hz);
    void noTone ();
    
  };  // end of TonePlayer
#endif