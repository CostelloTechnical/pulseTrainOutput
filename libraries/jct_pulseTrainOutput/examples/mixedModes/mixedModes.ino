/**
 * @file mixedModes.ino
 * @author CostelloTechnical
 * 
 * @brief This code generates 4 pulses on pin 11 of an Arduino Uno or Mega
 * and repeats it every three seconds.
 * 
 * It also generates a 750Hz continuous square wave on pin 9. Pin 9 is 
 * compatable with the Uno, but not the Mega. 
 * 
 * For a complete list of compatable pins for a given microcontroller, see the README file.
 * 
 * @date 2025-08-21
*/

#include "jct_pulseTrainOutput.h"

pulseTrainOutput pto1(11);     // Setting the pin that we want to generate pulses from.
pulseTrainOutput pto2(9);      // Setting the pin that we want to generate pulses from.
uint32_t repeatTimer_ms;       // A timer to stop the continuous wave after x milliseconds.

void setup() {
    pto1.generate(1000, DISCRETE, 4);  // Generate another 4 pulses at 1000Hz.
    pto2.generate(750);                // Generate pulses at 750Hz.
    repeatTimer_ms = millis();         // Set the repeat timer. 
}

void loop() {
    if(millis() - repeatTimer_ms > 3000){    // Has the repeat time elapsed.
        pto2.generate(1000, DISCRETE, 4);     // Generate another 4 pulses at 1000Hz.
        repeatTimer_ms = millis();           // Reset the timer.
    }
}