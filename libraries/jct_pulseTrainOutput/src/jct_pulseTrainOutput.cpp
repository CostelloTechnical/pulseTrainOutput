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
pulseTrainOutput* pulseTrainOutput::_instances[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};

// --- ISR Trampolines ---
// We now have one ISR for each timer. Each one calls the handler for its
// corresponding instance from the static array.

ISR(TIMER1_COMPA_vect) {
    if (pulseTrainOutput::_instances[TID_TIMER1] != nullptr) {
        pulseTrainOutput::_instances[TID_TIMER1]->handleInterrupt();
    }
}

ISR(TIMER2_COMPA_vect) {
    if (pulseTrainOutput::_instances[TID_TIMER2] != nullptr) {
        pulseTrainOutput::_instances[TID_TIMER2]->handleInterrupt();
    }
}

#if defined(__AVR_ATmega2560__) // Only compile these for the Mega
    ISR(TIMER3_COMPA_vect) {
        if (pulseTrainOutput::_instances[TID_TIMER3] != nullptr) {
            pulseTrainOutput::_instances[TID_TIMER3]->handleInterrupt();
        }
    }
    ISR(TIMER4_COMPA_vect) {
        if (pulseTrainOutput::_instances[TID_TIMER4] != nullptr) {
            pulseTrainOutput::_instances[TID_TIMER4]->handleInterrupt();
        }
    }
    ISR(TIMER5_COMPA_vect) {
        if (pulseTrainOutput::_instances[TID_TIMER5] != nullptr) {
            pulseTrainOutput::_instances[TID_TIMER5]->handleInterrupt();
        }
    }
#endif

// The constructor now does all the pin-to-timer mapping.
pulseTrainOutput::pulseTrainOutput(uint8_t pin) {
    _pin = pin;
    _isRunning = false;
    _timerId = TID_INVALID;

    // Use preprocessor directives to handle board differences.
    #if defined(__AVR_ATmega2560__) // Arduino Mega
        switch (_pin) {
            case 11: // Timer 1 A
                _timerId = TID_TIMER1;
                _is16bit = true;
                _tccrA = &TCCR1A;
                _tccrB = &TCCR1B;
                _timsk = &TIMSK1;
                _ocr = &OCR1A;
                _ocieBit = OCIE1A;
                _inputPort = &PINB;
                _pinBitMask = _BV(PB5);
                break;
            case 9:  // Timer 2 A
                _timerId = TID_TIMER2;
                _is16bit = false;
                _tccrA = &TCCR2A;
                _tccrB = &TCCR2B;
                _timsk = &TIMSK2;
                _ocr = (volatile uint16_t*)&OCR2A;
                _ocieBit = OCIE2A;
                _inputPort = &PINH;
                _pinBitMask = _BV(PH6);
                break;
            case 5:  // Timer 3 A
                _timerId = TID_TIMER3;
                _is16bit = true;
                _tccrA = &TCCR3A;
                _tccrB = &TCCR3B;
                _timsk = &TIMSK3;
                _ocr = &OCR3A;
                _ocieBit = OCIE3A;
                _inputPort = &PINE;
                _pinBitMask = _BV(PE3);
                break;
            case 6: // Timer 4 A
                _timerId = TID_TIMER4;
                _is16bit = true;
                _tccrA = &TCCR4A;
                _tccrB = &TCCR4B;
                _timsk = &TIMSK4;
                _ocr = &OCR4A;
                _ocieBit = OCIE4A;
                _inputPort = &PINH;
                _pinBitMask = _BV(PH3);
                break;
            case 46: // Timer 5 A
                _timerId = TID_TIMER5;
                _is16bit = true;
                _tccrA = &TCCR5A;
                _tccrB = &TCCR5B;
                _timsk = &TIMSK5;
                _ocr = &OCR5A; 
                _ocieBit = OCIE5A;
                _inputPort = &PINL;
                _pinBitMask = _BV(PL3);
                break;
        }
    #else // Arduino Uno, Nano, etc.
        switch (_pin) {
            case 9:  // Timer 1 A
                _timerId = TID_TIMER1;
                _is16bit = true;
                _tccrA = &TCCR1A;
                _tccrB = &TCCR1B;
                _timsk = &TIMSK1;
                _ocr = &OCR1A;
                _ocieBit = OCIE1A;
                _inputPort = &PINB;
                _pinBitMask = _BV(PB1);
                break;
            case 11: // Timer 2 A. Minimum frequency = 31Hz
                 _timerId = TID_TIMER2;
                 _is16bit = false;
                _tccrA = &TCCR2A;
                _tccrB = &TCCR2B;
                _timsk = &TIMSK2;
                _ocr = (volatile uint16_t*)&OCR2A;
                _ocieBit = OCIE2A;
                _inputPort = &PINB;
                _pinBitMask = _BV(PB3);
                break;
        }
    #endif

    if (_timerId != TID_INVALID) {
        _instances[_timerId] = this; // Register this instance to be handled by the correct ISR.
        pinMode(_pin, OUTPUT);
    }
}

bool pulseTrainOutput::generate(uint32_t frequency, pulseModes mode, uint32_t pulses) {
    if (_isRunning || _timerId == TID_INVALID) {
        return false;
    }
    _pulseMode = mode;
    if (mode == DISCRETE) {
        _pulsesToGenerate = pulses;
    }
    _pulseCounter = 0;

    // A frequency of 0 would lead to division by zero.
    if (frequency == 0) {
        return false;
    }

    cli();

    *_tccrA = 0;
    *_tccrB = 0;

    uint32_t ocrValue;
    uint8_t prescalerBits = 0;
    uint32_t maxOcr = _is16bit ? 0xFFFF : 0xFF;

    // --- New, more robust prescaler calculation logic ---
    if (_is16bit) { // For 16-bit timers (1, 3, 4, 5)
        ocrValue = (F_CPU / (2UL * frequency)) - 1;
        prescalerBits = _BV(CS10); // Prescaler 1
        if (ocrValue > maxOcr) {
            ocrValue = (F_CPU / (2UL * 8 * frequency)) - 1;
            prescalerBits = _BV(CS11); // Prescaler 8
        }
        if (ocrValue > maxOcr) {
            ocrValue = (F_CPU / (2UL * 64 * frequency)) - 1;
            prescalerBits = _BV(CS11) | _BV(CS10); // Prescaler 64
        }
        if (ocrValue > maxOcr) {
            ocrValue = (F_CPU / (2UL * 256 * frequency)) - 1;
            prescalerBits = _BV(CS12); // Prescaler 256
        }
        if (ocrValue > maxOcr) {
            ocrValue = (F_CPU / (2UL * 1024 * frequency)) - 1;
            prescalerBits = _BV(CS12) | _BV(CS10); // Prescaler 1024
        }
    } else { // For 8-bit Timer2
        ocrValue = (F_CPU / (2UL * frequency)) - 1;
        prescalerBits = _BV(CS20); // Prescaler 1
        if (ocrValue > maxOcr) {
            ocrValue = (F_CPU / (2UL * 8 * frequency)) - 1;
            prescalerBits = _BV(CS21); // Prescaler 8
        }
        if (ocrValue > maxOcr) {
            ocrValue = (F_CPU / (2UL * 32 * frequency)) - 1;
            prescalerBits = _BV(CS21) | _BV(CS20); // Prescaler 32
        }
        if (ocrValue > maxOcr) {
            ocrValue = (F_CPU / (2UL * 64 * frequency)) - 1;
            prescalerBits = _BV(CS22); // Prescaler 64
        }
        if (ocrValue > maxOcr) {
            ocrValue = (F_CPU / (2UL * 128 * frequency)) - 1;
            prescalerBits = _BV(CS22) | _BV(CS20); // Prescaler 128
        }
        if (ocrValue > maxOcr) {
            ocrValue = (F_CPU / (2UL * 256 * frequency)) - 1;
            prescalerBits = _BV(CS22) | _BV(CS21); // Prescaler 256
        }
        if (ocrValue > maxOcr) {
            ocrValue = (F_CPU / (2UL * 1024 * frequency)) - 1;
            prescalerBits = _BV(CS22) | _BV(CS21) | _BV(CS20); // Prescaler 1024
        }
    }
    
    // If the value is still too large, the frequency is out of range.
    if (ocrValue > maxOcr) {
        sei(); return false;
    }

    *_ocr = ocrValue;
    
    // Timer-Specific Configuration (this was the fix from last time)
    switch (_timerId) {
        case TID_TIMER1: case TID_TIMER3: case TID_TIMER4: case TID_TIMER5:
            *_tccrA |= _BV(COM1A0); *_tccrB |= _BV(WGM12);
            break;
        case TID_TIMER2:
            *_tccrA |= _BV(COM2A0) | _BV(WGM21);
            break;
    }
    
    *_timsk |= (1 << _ocieBit);
    _isRunning = true;
    *_tccrB |= prescalerBits;

    sei();
    return true;
}

void pulseTrainOutput::stop() {
    // We must also use the correct prescaler bits to stop the timer.
    switch (_timerId) {
        case TID_TIMER1:
        case TID_TIMER3:
        case TID_TIMER4:
        case TID_TIMER5:
            *_tccrB &= ~(_BV(CS12) | _BV(CS11) | _BV(CS10));
            break;
        case TID_TIMER2:
            *_tccrB &= ~(_BV(CS22) | _BV(CS21) | _BV(CS20));
            break;
    }
    *_timsk &= ~(1 << _ocieBit); // Disable interrupt
    _isRunning = false;
}

void pulseTrainOutput::handleInterrupt() {
    if(_pulseMode == DISCRETE){
      uint8_t portState = *_inputPort;
        _pulseCounter += (*_inputPort & _pinBitMask)==_pinBitMask;
        if (_pulseCounter >= _pulsesToGenerate) {
            if((*_inputPort & _pinBitMask) == 0){
                stop();
            }
        }
    }
}

bool pulseTrainOutput::isRunning() {
    return _isRunning;
}