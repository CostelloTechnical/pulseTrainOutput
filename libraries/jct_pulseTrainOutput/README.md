# JCT Arduino Hardware Pulse Train Library

A high-performance Arduino library for generating precise discrete or continuous hardware-timed pulse trains on Uno and Mega boards.

This library allows for direct control of the AVR microcontroller's hardware timers to produce highly accurate, non-blocking square waves. It is designed to be efficient and flexible, supporting multiple concurrent pulse trains on different timer-enabled pins.

## Key Features

* Hardware-Timed Precision: Bypasses the standard Arduino tone() function for direct, low-level register control.
* Non-Blocking Operation: Once started, the hardware generates the waveform, leaving the CPU free to perform other tasks.
* Multiple Modes:
    * CONTINUOUS: Generate an unending square wave.
    * DISCRETE: Generate a precise number of pulses and then automatically stop.
* Multi-Channel Support: Run multiple, independent pulse trains simultaneously on different hardware timers (up to 2 on Uno, up to 5 on Mega).
* Dynamic Frequency Updates: Change the frequency of a running wave "on the fly" for effects like sirens or frequency sweeps.

## Hardware Support & Pinout

This library is board-aware and will automatically use the correct timer for the pin you select.

### Arduino Uno

| Pin  | Timer  | Channel | Notes                    |
| :--- | :----- | :------- | :----------------------- |
| 9  | Timer1 | A        | 16-bit, high resolution. |
| 11 | Timer2 | A        | 8-bit.                   |

### Arduino Mega 2560

| Pin  | Timer  | Channel | Notes     |
| :--- | :----- | :------- | :-------- |
| 11 | Timer1 | A        | 16-bit.   |
| 10 | Timer2 | A        | 8-bit.    |
| 5  | Timer3 | A        | 16-bit.   |
| 6  | Timer4 | A        | 16-bit.   |
| 46 | Timer5 | A        | 16-bit.   |

Note: Timer0 pins and Channel B/C pins are currently unsupported to maintain simplicity and avoid conflicts with core Arduino timing functions like millis() and delay().

## Installation

1.  Download the library files (jct_pulseTrainOutput.h and jct_pulseTrainOutput.cpp).
2.  In the Arduino IDE, go to Sketch > Show Sketch Folder.
3.  Create a folder named libraries inside your sketch folder if it doesn't already exist.
4.  Create a folder named jct_pulseTrainOutput inside the libraries folder.
5.  Place the .h and .cpp files inside the jct_pulseTrainOutput folder.
6.  Restart the Arduino IDE.

## Basic Usage (Quick Start)

Here is a minimal example to generate a continuous 1 kHz tone on Pin 9 of an Arduino Uno.

cpp #include "jct_pulseTrainOutput.h"  // Create an object for a timer-enabled pin. 
pulseTrainOutput pto(9); 
void setup() {   // The constructor handles all hardware setup.   
// No begin() method is needed.      
// Start a continuous 1000 Hz square wave.   
pto.generate(1000); }  
void loop() {   // The hardware generates the wave, so the loop is free } 

## API Reference

A brief overview of the main public functions.

* pulseTrainOutput(uint8_t pin)  :  Constructor. Creates a generator object and sets up the hardware for a specific pin.
* isValid()  :  Returns true if the pin chosen in the constructor is valid.
* generate(frequency, mode, pulses) :  Starts a pulse train. mode can be DISCRETE or CONTINUOUS. pulses is only used in DISCRETE mode.
* stop()  :  Immediately stops the pulse train and forces the pin LOW.
* isRunning()  :  Returns true if the timer is currently active.
* updateFrequency(newFrequency) :  Updates the frequency if opperating in continuous mode.

## License

This software is provided "as is", without warranty of any kind. Permission is hereby granted, free of charge, to any person obtaining a copy of this software to deal in the Software without restriction.

It is highly encouraged that if you find this library useful, you provide attribution back to the original author.
