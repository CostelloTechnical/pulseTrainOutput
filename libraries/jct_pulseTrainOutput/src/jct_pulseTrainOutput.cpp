/**
 * @file jct_pulseTrainOutput.cpp
 * @author CostelloTechnical
 * @brief Source file for the jct_pulseTrainOutput library.
 * This library provides a C++ class to generate precise, hardware-timed
 * pulse trains on various output pins of Arduino Uno, Mega, and R4 boards.
 * @version 1.5
 * @date 2025-08-27
 */
#include "jct_pulseTrainOutput.h"

// Initialize the static array of pointers.
pulseTrainOutput* pulseTrainOutput::_instances[10] = {nullptr};

#if defined(ARDUINO_UNOR4_MINIMA) || defined(ARDUINO_UNOR4_WIFI)

// --- UNO R4 IMPLEMENTATION ---

// Define a unique callback for each timer channel. Each callback knows its channel
// number and calls the appropriate instance's handler. This bypasses the p_context issue.
void pulseTrainOutput::r4_gpt_callback_0(timer_callback_args_t *) { 
    if (_instances[0]) _instances[0]->handleInterrupt(); 
}
void pulseTrainOutput::r4_gpt_callback_1(timer_callback_args_t *) { 
    if (_instances[1]) _instances[1]->handleInterrupt(); 
}
void pulseTrainOutput::r4_gpt_callback_2(timer_callback_args_t *) {
    if (_instances[2]) _instances[2]->handleInterrupt();
}
void pulseTrainOutput::r4_gpt_callback_3(timer_callback_args_t *) {
    if (_instances[3]) _instances[3]->handleInterrupt();
}
void pulseTrainOutput::r4_gpt_callback_4(timer_callback_args_t *) {
    if (_instances[4]) _instances[4]->handleInterrupt();
}
void pulseTrainOutput::r4_gpt_callback_5(timer_callback_args_t *) {
    if (_instances[5]) _instances[5]->handleInterrupt();
}
void pulseTrainOutput::r4_gpt_callback_6(timer_callback_args_t *) {
    if (_instances[6]) _instances[6]->handleInterrupt();
}
void pulseTrainOutput::r4_gpt_callback_7(timer_callback_args_t *) {
    if (_instances[7]) _instances[7]->handleInterrupt();
}

// ** FIX #1: Use the explicit function pointer type instead of the inaccessible 'timer_callback_t' typedef. **
// This creates an array of pointers to functions that return void and accept a timer_callback_args_t*.
static void (*r4_callbacks[])(timer_callback_args_t*) = {
  pulseTrainOutput::r4_gpt_callback_0,
  pulseTrainOutput::r4_gpt_callback_1,
  pulseTrainOutput::r4_gpt_callback_2,
  pulseTrainOutput::r4_gpt_callback_3,
  pulseTrainOutput::r4_gpt_callback_4,
  pulseTrainOutput::r4_gpt_callback_5,
  pulseTrainOutput::r4_gpt_callback_6,
  pulseTrainOutput::r4_gpt_callback_7
};

#else

// --- AVR IMPLEMENTATION (UNO R3, MEGA, etc.) ---

ISR(TIMER1_COMPA_vect) { if (pulseTrainOutput::_instances[TID_TIMER1] != nullptr) { pulseTrainOutput::_instances[TID_TIMER1]->handleInterrupt(); } }
ISR(TIMER2_COMPA_vect) { if (pulseTrainOutput::_instances[TID_TIMER2] != nullptr) { pulseTrainOutput::_instances[TID_TIMER2]->handleInterrupt(); } }
#if defined(__AVR_ATmega2560__)
ISR(TIMER3_COMPA_vect) { if (pulseTrainOutput::_instances[TID_TIMER3] != nullptr) { pulseTrainOutput::_instances[TID_TIMER3]->handleInterrupt(); } }
ISR(TIMER4_COMPA_vect) { if (pulseTrainOutput::_instances[TID_TIMER4] != nullptr) { pulseTrainOutput::_instances[TID_TIMER4]->handleInterrupt(); } }
ISR(TIMER5_COMPA_vect) { if (pulseTrainOutput::_instances[TID_TIMER5] != nullptr) { pulseTrainOutput::_instances[TID_TIMER5]->handleInterrupt(); } }
#endif

#endif // End of platform-specific ISR/callback section


pulseTrainOutput::pulseTrainOutput(uint8_t pin) {
    _pin = pin;
    _timerId = TID_INVALID; 
    _isRunning = false;
    _error = 0;

#if defined(ARDUINO_UNOR4_MINIMA) || defined(ARDUINO_UNOR4_WIFI)
    auto pin_cgf = getPinCfgs(_pin, PIN_CFG_REQ_PWM);
    if (pin_cgf[0] == 0) {
        _error = INVALID_PIN;
        return;
    }
    _timer_channel = GET_CHANNEL(pin_cgf[0]);
    _is_agt = IS_PIN_AGT_PWM(pin_cgf[0]);
    _pwm_channel = IS_PWM_ON_A(pin_cgf[0]) ? CHANNEL_A : CHANNEL_B;
      R_IOPORT_PinCfg(&g_ioport_ctrl, g_pin_cfg[_pin].pin, (uint32_t)(IOPORT_CFG_PERIPHERAL_PIN | (_is_agt ? IOPORT_PERIPHERAL_AGT : IOPORT_PERIPHERAL_GPT1)));
    _timerId = (timerIds)_timer_channel;
    if (_timerId != TID_INVALID) {
      _instances[_timer_channel] = this;
    } else {
      _error = INVALID_PIN;
    }
#else
    #if defined(__AVR_ATmega2560__)
        switch (_pin) {
            case 11: _timerId = TID_TIMER1; _is16bit = true; _tccrA = &TCCR1A; _tccrB = &TCCR1B; _timsk = &TIMSK1; _ocr = &OCR1A; _ocieBit = OCIE1A; _outputPort = &PORTB; _pinBitMask = _BV(PB5); _comStopMask = _BV(COM1A1) | _BV(COM1A0); break;
            case 10: _timerId = TID_TIMER2; _is16bit = false; _tccrA = &TCCR2A; _tccrB = &TCCR2B; _timsk = &TIMSK2; _ocr = (volatile uint16_t*)&OCR2A; _ocieBit = OCIE2A; _outputPort = &PORTB; _pinBitMask = _BV(PB4); _comStopMask = _BV(COM2A1) | _BV(COM2A0); break;
            case 5: _timerId=TID_TIMER3; _is16bit=true; _tccrA=&TCCR3A; _tccrB=&TCCR3B; _timsk=&TIMSK3; _ocr=&OCR3A; _ocieBit=OCIE3A; _outputPort=&PORTE; _pinBitMask=_BV(PE3); _comStopMask=_BV(COM3A1)|_BV(COM3A0); break;
            case 6: _timerId=TID_TIMER4; _is16bit=true; _tccrA=&TCCR4A; _tccrB=&TCCR4B; _timsk=&TIMSK4; _ocr=&OCR4A; _ocieBit=OCIE4A; _outputPort=&PORTH; _pinBitMask=_BV(PH3); _comStopMask=_BV(COM4A1)|_BV(COM4A0); break;
            case 46: _timerId=TID_TIMER5; _is16bit=true; _tccrA=&TCCR5A; _tccrB=&TCCR5B; _timsk=&TIMSK5; _ocr=&OCR5A; _ocieBit=OCIE5A; _outputPort=&PORTL; _pinBitMask=_BV(PL3); _comStopMask=_BV(COM5A1)|_BV(COM5A0); break;
        }
    #else // Arduino Uno, Nano, etc.
        switch (_pin) {
            case 9: _timerId=TID_TIMER1; _is16bit=true; _tccrA=&TCCR1A; _tccrB=&TCCR1B; _timsk=&TIMSK1; _ocr=&OCR1A; _ocieBit=OCIE1A; _outputPort=&PORTB; _pinBitMask=_BV(PB1); _comStopMask=_BV(COM1A1)|_BV(COM1A0); break;
            case 11: _timerId=TID_TIMER2; _is16bit=false; _tccrA=&TCCR2A; _tccrB=&TCCR2B; _timsk=&TIMSK2; _ocr=(volatile uint16_t*)&OCR2A; _ocieBit=OCIE2A; _outputPort=&PORTB; _pinBitMask=_BV(PB3); _comStopMask=_BV(COM2A1)|_BV(COM2A0); break;
        }
    #endif
    if (_timerId != TID_INVALID) {
        _instances[_timerId] = this;
        pinMode(_pin, OUTPUT);
    } else {
      _error = INVALID_PIN;
    }
#endif
}

bool pulseTrainOutput::generate(uint32_t frequency, pulseModes mode, uint32_t pulses) {
    if (_isRunning || _timerId == TID_INVALID || frequency == 0 || mode > CONTINUOUS || mode <= STOP) {
        _error = _isRunning ? ACTIVE : _error;
        _error = frequency == 0 ? ZERO_HZ : _error;
        _error = _timerId == TID_INVALID ? INVALID_PIN : _error;
        _error = mode > CONTINUOUS ? INVALID_MODE : _error;
        _error = mode <= STOP ? INVALID_MODE : _error;
        _error = (pulses == 0 && mode == DISCRETE) ? ZERO_PULSES : _error;
        return false;
    }

    _pulseMode = mode;

#if defined(ARDUINO_UNOR4_MINIMA) || defined(ARDUINO_UNOR4_WIFI)
    // --- UNO R4 Generate Logic ---
    if (_pulseMode == DISCRETE) {
        _pulsesToGenerate = pulses;
        _pulseCounter = _pulsesToGenerate;
    }
    
    // Select the correct hardcoded callback based on the timer channel for this pin.
    void (*selected_callback)(timer_callback_args_t*) = nullptr;
    if (_timer_channel < (sizeof(r4_callbacks) / sizeof(r4_callbacks[0]))) {
      selected_callback = r4_callbacks[_timer_channel];
    }

    // We must use the .begin() method to register the callback.
    _timer.begin(TIMER_MODE_PWM, _is_agt, _timer_channel, frequency, 50, selected_callback);
    
    if (mode == DISCRETE) {
        // ** FIX #2: Call setup_overflow_irq() with NO arguments. **
        // This enables the interrupt for the callback that was already registered in .begin().
        _timer.setup_overflow_irq();
    }
    _timer.add_pwm_extended_cfg();
    _timer.enable_pwm_channel(_pwm_channel);
     // --- ADD THIS CHECK ---
    if (!_timer.open()) {
        // If open() returns false, the hardware setup failed.
        _error = 7; // Let's use 7 as a custom error for "TIMER_OPEN_FAILED"
        _isRunning = false;
        return false;
    }
    // --- END OF CHECK ---

    _timer.start();
    _isRunning = true;
    return true;

#else
    // --- AVR Generate Logic ---
    if (_pulseMode == DISCRETE) {
        _pulsesToGenerate = pulses * 2;
        _pulseCounter = _pulsesToGenerate;
    }
    uint32_t ocrValue;
    uint8_t prescalerBits;
    if (!_calculateTimingParameters(frequency, ocrValue, prescalerBits)) {
        return false;
    }
    cli();

    *_tccrA = 0;
    *_tccrB = 0;
    *_ocr = ocrValue;
    switch (_timerId) {
        case TID_TIMER1: case TID_TIMER3: case TID_TIMER4: case TID_TIMER5:
            *_tccrA |= _BV(COM1A0); *_tccrB |= _BV(WGM12); break;
        case TID_TIMER2:
            *_tccrA |= _BV(COM2A0) | _BV(WGM21); break;
    }
    if (mode == DISCRETE) {
        *_timsk |= (1 << _ocieBit);
    }
    _isRunning = true;
    *_tccrB |= prescalerBits;
    sei();
    return true;
#endif
}

bool pulseTrainOutput::updateFrequency(uint32_t newFrequency) {
    if (!_isRunning || newFrequency == 0) {
        return false;
    }
#if defined(ARDUINO_UNOR4_MINIMA) || defined(ARDUINO_UNOR4_WIFI)
    bool success = _timer.set_frequency(newFrequency);
    if (success) {
        // IMPORTANT: The FspTimer::set_frequency function does not automatically
        // update the duty cycle to match the new period. We must manually
        // re-apply our desired 50% duty cycle. This forces the library to
        // recalculate the compare value based on the new period.
        uint32_t period_counts = _timer.get_period_raw(); 

        // Calculate the new duty cycle for 50%
        uint32_t duty_counts = period_counts / 2;

        // Set the new duty cycle in raw counts
        success = _timer.set_duty_cycle(duty_counts, _pwm_channel);
    }
    return success;
#else
    uint32_t ocrValue;
    uint8_t prescalerBits;
    if (!_calculateTimingParameters(newFrequency, ocrValue, prescalerBits)) {
        return false;
    }
    cli();
    uint8_t tccrb_cache = *_tccrB; 
    *_tccrB &= ~(_BV(CS12) | _BV(CS11) | _BV(CS10));
    *_ocr = ocrValue;
    *_tccrB = (tccrb_cache & ~(_BV(CS12) | _BV(CS11) | _BV(CS10))) | prescalerBits;
    sei();
    return true;
#endif
}

#if defined(ARDUINO_UNOR4_MINIMA) || defined(ARDUINO_UNOR4_WIFI)
void pulseTrainOutput::stop() {
    if (!_isRunning) return;
    _timer.stop();
    _timer.end();
    _isRunning = false;
}
#else
void pulseTrainOutput::stop() {
    *_tccrA &= ~_comStopMask;
    *_outputPort &= ~_pinBitMask;
    switch (_timerId) {
        case TID_TIMER1: case TID_TIMER3: case TID_TIMER4: case TID_TIMER5:
            *_tccrB &= ~(_BV(CS12) | _BV(CS11) | _BV(CS10)); break;
        case TID_TIMER2:
            *_tccrB &= ~(_BV(CS22) | _BV(CS21) | _BV(CS20)); break;
    }
    *_timsk &= ~(1 << _ocieBit);
    _isRunning = false;
}
#endif

#if defined(ARDUINO_UNOR4_MINIMA) || defined(ARDUINO_UNOR4_WIFI)
void pulseTrainOutput::handleInterrupt() {
        if (_pulseMode == DISCRETE) {
            if (_pulseCounter == 0) {
                stop();
            }
            if (_pulseCounter > 0) {
                _pulseCounter--;
                if (_pulseCounter == 0) {
                    // Instead of stopping, we command the PWM to be silent for the next full cycle.
                    // This holds the output pin low, creating our "final off cycle".
                    _timer.set_duty_cycle(0, _pwm_channel);
                }
            }
        }
    }
#else
void pulseTrainOutput::handleInterrupt() {
        if (_pulseMode == DISCRETE) {
            _pulseCounter--;
            if (_pulseCounter == 0) {
                stop();
            }
        }
    }
#endif


bool pulseTrainOutput::isRunning() const{
    return _isRunning;
}

uint8_t pulseTrainOutput::getError() const{
    return _error;
}

#if !defined(ARDUINO_UNOR4_MINIMA) && !defined(ARDUINO_UNOR4_WIFI)
bool pulseTrainOutput::_calculateTimingParameters(uint32_t frequency, uint32_t& ocrValue, uint8_t& prescalerBits) {
    uint32_t maxOcr = _is16bit ? 0xFFFF : 0xFF;
    if (_is16bit) {
        ocrValue = (F_CPU / (2UL * frequency)) - 1; prescalerBits = _BV(CS10);
        if (ocrValue > maxOcr) { ocrValue = (F_CPU / (2UL * 8 * frequency)) - 1; prescalerBits = _BV(CS11); }
        if (ocrValue > maxOcr) { ocrValue = (F_CPU / (2UL * 64 * frequency)) - 1; prescalerBits = _BV(CS11) | _BV(CS10); }
        if (ocrValue > maxOcr) { ocrValue = (F_CPU / (2UL * 256 * frequency)) - 1; prescalerBits = _BV(CS12); }
        if (ocrValue > maxOcr) { ocrValue = (F_CPU / (2UL * 1024 * frequency)) - 1; prescalerBits = _BV(CS12) | _BV(CS10); }
    } else {
        ocrValue = (F_CPU / (2UL * frequency)) - 1; prescalerBits = _BV(CS20);
        if (ocrValue > maxOcr) { ocrValue = (F_CPU / (2UL * 8 * frequency)) - 1; prescalerBits = _BV(CS21); }
        if (ocrValue > maxOcr) { ocrValue = (F_CPU / (2UL * 32 * frequency)) - 1; prescalerBits = _BV(CS21) | _BV(CS20); }
        if (ocrValue > maxOcr) { ocrValue = (F_CPU / (2UL * 64 * frequency)) - 1; prescalerBits = _BV(CS22); }
        if (ocrValue > maxOcr) { ocrValue = (F_CPU / (2UL * 128 * frequency)) - 1; prescalerBits = _BV(CS22) | _BV(CS20); }
        if (ocrValue > maxOcr) { ocrValue = (F_CPU / (2UL * 256 * frequency)) - 1; prescalerBits = _BV(CS22) | _BV(CS21); }
        if (ocrValue > maxOcr) { ocrValue = (F_CPU / (2UL * 1024 * frequency)) - 1; prescalerBits = _BV(CS22) | _BV(CS21) | _BV(CS20); }
    }
    if (ocrValue > maxOcr) {
        _error = FREQUENCY_HIGH;
        return false;
    }
    return true;
}
#else
bool pulseTrainOutput::_calculateTimingParameters(uint32_t, uint32_t&, uint8_t&) {
    return false;
}
#endif