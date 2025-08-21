/* 
* @file jct_pulseTrainOutput.cpp
 * @author CostelloTechnical
 * @brief Header file for the jct_pulseTrainOutput library.
 * This library provides a C++ class to generate precise, hardware-timed
 * pulse trains on various output pins of Arduino Uno and Mega boards.
 * @version 1.2
 * @date 2025-08-21
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

  ==============================================================================
                            INFORMATION ON USE
  ==============================================================================

  UNO
    Acceptable pins: D9 and D11.
    The minimum frequency on D11 is 31Hz. 16MHz / (2 * 1024 * 256) ≈ 30.5 Hz.
    The maximum usable discrete frequency is 60kHz with one channel. With two 
    channels that is lowered to 30kHz per channel. This is a fundamental hardware
    limit. You can however run one channel in DISCRETE at 60kHz and the other at
    up to 8MHz in CONTINUOUS with no problems (other than 8MHz being a bit messy).
 
  MEGA
    Acceptable pins: D5, D6, D10, D11 and D46.
    The minimum frequency on D10 is 31Hz. 16MHz / (2 * 1024 * 256) ≈ 30.5 Hz.
    The maximum usable discrete frequency is 55kHz with one channel (at least
    on my Mega). With all 5 timers on at the same time I can only generate 7.5kHz
    reliably. All timers can generate up to 8MHz simultaneously.
*/
#include "jct_pulseTrainOutput.h"
// Initialize the static array of pointers. This is crucial for the ISR trampolines.
pulseTrainOutput* pulseTrainOutput::_instances[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};

// --- ISR Trampolines ---
// These are the global, C-style Interrupt Service Routines.
// The hardware can only call these simple functions. Their only job is to
// act as a "trampoline" to bounce the execution to the correct C++ object's
// handleInterrupt() method.

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

/**
 * @brief Construct a new pulseTrainOutput object.
 * This constructor performs all the necessary board- and pin-specific hardware mapping.
 * It identifies the correct timer, registers, and configuration bits for the given pin
 * and stores them in the object's member variables.
 * @param pin The Arduino digital pin to control.
 */
pulseTrainOutput::pulseTrainOutput(uint8_t pin) {
    _pin = pin;
    _timerId = TID_INVALID; // Default to invalid
    _isRunning = false;
    _error = 0;
        #if defined(__AVR_ATmega2560__) // Arduino Mega
        switch (_pin) {
            case 11: // Timer 1 (16-bit), Channel A
                _timerId = TID_TIMER1;
                _is16bit = true;
                _tccrA = &TCCR1A; // Control Register A: Controls pin action (COM bits)
                _tccrB = &TCCR1B; // Control Register B: Controls mode (WGM bits) and clock speed (CS bits)
                _timsk = &TIMSK1; // Interrupt Mask Register: Enables interrupts for this timer
                _ocr = &OCR1A;    // Output Compare Register A: The target value for the counter
                _ocieBit = OCIE1A;// Interrupt Enable bit for channel A
                _outputPort = &PORTB; // The physical port for this pin
                _pinBitMask = _BV(PB5); // The bitmask for this pin within the port
                _comStopMask = _BV(COM1A1) | _BV(COM1A0); // Mask to clear the COM bits and disconnect the pin
                break;
            case 10: // Timer 2, Channel A
                _timerId = TID_TIMER2;
                _is16bit = false;
                _tccrA = &TCCR2A;
                _tccrB = &TCCR2B;
                _timsk = &TIMSK2;
                _ocr = (volatile uint16_t*)&OCR2A;
                _ocieBit = OCIE2A;
                _outputPort = &PORTB;
                _pinBitMask = _BV(PB4);
                _comStopMask = _BV(COM2A1) | _BV(COM2A0);
                break;
            case 5: // Timer 3, Channel A
                _timerId=TID_TIMER3;
                _is16bit=true;
                _tccrA=&TCCR3A;
                _tccrB=&TCCR3B;
                _timsk=&TIMSK3;
                _ocr=&OCR3A;
                _ocieBit=OCIE3A;
                _outputPort=&PORTE;
                _pinBitMask=_BV(PE3);
                _comStopMask=_BV(COM3A1)|_BV(COM3A0);
                break;
            case 6:// Timer 4, Channel A
                _timerId=TID_TIMER4;
                _is16bit=true;
                _tccrA=&TCCR4A;
                _tccrB=&TCCR4B;
                _timsk=&TIMSK4;
                _ocr=&OCR4A;
                _ocieBit=OCIE4A;
                _outputPort=&PORTH;
                _pinBitMask=_BV(PH3);
                _comStopMask=_BV(COM4A1)|_BV(COM4A0);
                break;
            case 46: // Timer 5, Channel A
                _timerId=TID_TIMER5;
                _is16bit=true;
                _tccrA=&TCCR5A;
                _tccrB=&TCCR5B;
                _timsk=&TIMSK5;
                _ocr=&OCR5A;
                _ocieBit=OCIE5A;
                _outputPort=&PORTL;
                _pinBitMask=_BV(PL3);
                _comStopMask=_BV(COM5A1)|_BV(COM5A0);
                break;
        }
    #else // Arduino Uno, Nano, etc.
        switch (_pin) {
            case 9: // Timer 1, Channel A
                _timerId=TID_TIMER1;
                _is16bit=true;
                _tccrA=&TCCR1A;
                _tccrB=&TCCR1B;
                _timsk=&TIMSK1;
                _ocr=&OCR1A;
                _ocieBit=OCIE1A;
                _outputPort=&PORTB;
                _pinBitMask=_BV(PB1);
                _comStopMask=_BV(COM1A1)|_BV(COM1A0);
                break;
            case 11:  // Timer 2, Channel A
                _timerId=TID_TIMER2;
                _is16bit=false;
                _tccrA=&TCCR2A;
                _tccrB=&TCCR2B;
                _timsk=&TIMSK2;
                _ocr=(volatile uint16_t*)&OCR2A;
                _ocieBit=OCIE2A;
                _outputPort=&PORTB;
                _pinBitMask=_BV(PB3);
                _comStopMask=_BV(COM2A1)|_BV(COM2A0);
                break;
        }
    #endif

    // If the pin was valid and a timer was assigned...
    if (_timerId != TID_INVALID) {
        // ...register this object instance in the static array for its timer.
        _instances[_timerId] = this;
        // And set the pin as an output.
        pinMode(_pin, OUTPUT);
    }
}

/**
 * @brief The main "workhorse" function to start a pulse train.
 */
bool pulseTrainOutput::generate(uint32_t frequency, pulseModes mode, uint32_t pulses) {
    // 1. --- Safety Checks ---
    if (_isRunning || _timerId == TID_INVALID || frequency == 0) {
        _error = _isRunning ? ACTIVE : _error;
        _error = frequency == 0 ? ZERO_HZ : _error;
        _error = _timerId == TID_INVALID ? INVALID_PIN : _error;
        return false;
    }

    // 2. --- Set Internal State ---
    _pulseMode = mode;
    if (_pulseMode == DISCRETE) {
        // For N high pulses finishing LOW, we need 2*N total toggles.
        _pulsesToGenerate = pulses * 2;
        _pulseCounter = 0;
    }
     // Disable global interrupts during hardware configuration to prevent issues.
    cli();

    // Reset timer control registers to a clean slate.
    *_tccrA = 0;
    *_tccrB = 0;

    uint32_t ocrValue;
    uint8_t prescalerBits = 0;
    uint32_t maxOcr = _is16bit ? 0xFFFF : 0xFF;

    // 3. --- Prescaler and OCR Calculation Logic ---
    // This block tries each prescaler from fastest to slowest until it finds one
    // that allows the calculated OCR value to fit within the timer's bit-depth.
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

    // 4. --- Write Configuration to Hardware Registers ---
    *_ocr = ocrValue;
    
    // This generic logic works because the constructor stored the correct bit names.
    switch (_timerId) {
        case TID_TIMER1: case TID_TIMER3: case TID_TIMER4: case TID_TIMER5:
            *_tccrA |= _BV(COM1A0);  // Set Toggle on Compare Match
            *_tccrB |= _BV(WGM12);   // Set CTC mode
            break;
        case TID_TIMER2:
            *_tccrA |= _BV(COM2A0) | _BV(WGM21); // Set Toggle and CTC mode for Timer2
            break;
    }
    
    // 5. --- Enable Interrupts and Start Timer ---
    if (mode == DISCRETE) {
        *_timsk |= (1 << _ocieBit); // Enable interrupt only if needed.
    }
    _isRunning = true;
    *_tccrB |= prescalerBits;

    sei(); // Re-enable global interrupts.
    return true;
}

/**
 * @brief Stops the timer and ensures the pin is left in a LOW state.
 */
void pulseTrainOutput::stop() {
    *_tccrA &= ~_comStopMask; // Instantly disconnect pin
    *_outputPort &= ~_pinBitMask; // Force pin LOW

    switch (_timerId) {
        case TID_TIMER1: case TID_TIMER3: case TID_TIMER4: case TID_TIMER5:
            *_tccrB &= ~(_BV(CS12) | _BV(CS11) | _BV(CS10)); break;
        case TID_TIMER2:
            *_tccrB &= ~(_BV(CS22) | _BV(CS21) | _BV(CS20)); break;
    }
    
    *_timsk &= ~(1 << _ocieBit);
    _isRunning = false;
}

/**
 * @brief The ISR handler. It increments the pulse counter and stops when the target is reached.
 */
void pulseTrainOutput::handleInterrupt() {
    if (_pulseMode == DISCRETE) {
        _pulseCounter++;
        if (_pulseCounter >= _pulsesToGenerate) {
            stop();
        }
    }
}

/**
 * @brief Checks if the timer is running.
 * @return true if running, false otherwise.
 */
bool pulseTrainOutput::isRunning() const{
    return _isRunning;
}

/**
 * @brief Checks if there was an error with generating.
 * @return Returns the current error, if any.
 */
uint8_t pulseTrainOutput::getError() const{
    return _error;
}