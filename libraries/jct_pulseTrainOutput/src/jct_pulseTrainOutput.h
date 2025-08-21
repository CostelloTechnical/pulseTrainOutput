/*
 * @file jct_pulseTrainOutput.h
 * @author CostelloTechnical
 * @brief Header file for the jct_pulseTrainOutput library.
 * This library provides a C++ class to generate precise, hardware-timed
 * pulse trains on various output pins of Arduino Uno and Mega boards.
 * @version 1.1
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

*/
/**
 * @brief Defines the operating modes for the pulse generator.
 */
#ifndef JCT_PULSETRAINOUTPUT_H
#define JCT_PULSETRAINOUTPUT_H
#include <Arduino.h>

enum pulseModes {
    STOP = 0,       // Not used, represents the stopped state.
    DISCRETE = 1,   // Generate a specific number of pulses and then stop.
    CONTINUOUS = 2  // Generate a continuous, unending wave.
};

/**
 * @brief Internal enum to identify the specific hardware timer being used.
 */
enum timerIds {
    TID_TIMER1,
    TID_TIMER2,
    TID_TIMER3,
    TID_TIMER4,
    TID_TIMER5,
    TID_INVALID
};

/**
 * @brief A C++ class to control Arduino hardware timers for precise pulse/frequency generation.
 * * This class abstracts the low-level timer registers of the AVR microcontroller,
 * allowing for the generation of multiple, simultaneous pulse trains on different
 * timer-enabled pins on both Arduino Uno and Mega boards.
 */
class pulseTrainOutput{
  public:
    /**
     * @brief An array of static pointers, one for each timer, allowing the global C-style
     * ISRs to find and call the correct C++ object instance.
     */
    static pulseTrainOutput* _instances[5];

    /**
     * @brief Construct a new pulseTrainOutput object.
     * @param pin The Arduino digital pin to control. Must be a timer-enabled pin.
     */
    pulseTrainOutput(uint8_t pin);

    /**
     * @brief Generates a pulse train with the specified parameters.
     * @param frequency The desired frequency of the square wave in Hertz.
     * @param mode The operating mode (DISCRETE or CONTINUOUS).
     * @param pulses The number of HIGH pulses to generate in DISCRETE mode. Ignored in CONTINUOUS mode.
     * @return true if the frequency is achievable and the timer was started.
     * @return false if the pin is invalid, the timer is already running, or the frequency is out of range.
     */
    bool generate(uint32_t frequency, pulseModes mode = CONTINUOUS, uint32_t pulses = 1);
    
    /**
     * @brief Immediately stops the pulse train generation.
     */ 
    void stop();

    /**
     * @brief Checks if the timer is currently generating pulses.
     * @return true if the timer is active, false otherwise.
     */
    bool isRunning() const;

     /**
     * @brief The C++ interrupt handler method. This is called by the global ISR trampolines.
     * It contains the logic for counting discrete pulses.
     */
    void handleInterrupt();

private:
     // --- Instance Members ---
    uint8_t _pin;                         // The Arduino pin number this object controls.
    timerIds _timerId;                    // The hardware timer this instance is mapped to (e.g., TID_TIMER1).
    bool _is16bit;                        // Flag to handle 8-bit vs 16-bit timer differences.


    // --- Pointers to Hardware Registers ---
    // These are populated by the constructor based on the chosen pin.
    volatile uint8_t* _tccrA;             // Pointer to the Timer/Counter Control Register A (e.g., TCCR1A). Controls pin action (COM bits) and mode (WGM bits).
    volatile uint8_t* _tccrB;             // Pointer to the Timer/Counter Control Register B (e.g., TCCR1B). Controls mode (WGM bits) and clock speed (CS bits).
    volatile uint8_t* _timsk;             // Pointer to the Timer Interrupt Mask Register (e.g., TIMSK1). Enables/disables timer-specific interrupts.
    volatile uint16_t* _ocr;              // Pointer to the Output Compare Register (e.g., OCR1A). This is the target value the timer counts to.
    volatile uint8_t* _outputPort;        // Pointer to the physical PORTx register for this pin (e.g., PORTB). Used for forcing the pin LOW on stop.


    // --- Stored Configuration Bits ---
    // These are determined in the constructor to make the generate() and stop() methods generic.
    uint8_t _ocieBit;                     // The specific interrupt enable bit for this timer/channel (e.g., OCIE1A).
    uint8_t _comStopMask;                 // The bitmask used to disconnect the pin from the timer.


    // --- State variables ---
    volatile uint32_t _pulseCounter;      // Tracks the number of pulses generated in DISCRETE mode. Must be volatile as it's modified in an ISR.
    volatile uint32_t _pulsesToGenerate;  // The target number of pulses in DISCRETE mode. Volatile as it's read in an ISR.
    volatile uint8_t _pulseMode;          // The current operating mode. Volatile as it's read in an ISR.
    volatile bool _isRunning;             // The current running state. Volatile as it's modified in an ISR.
    uint8_t _pinBitMask;                  // The bitmask for this pin within its PORT (e.g., _BV(PB5)).

};

#endif // JCT_PULSETRAINOUTPUT_H