/**
 * @file discretePulses.ino
 * @author CostelloTechnical
 * 
 * @brief This code generates 4 pulses on pin 11 of an Arduino Uno or Mega
 * and repeats it every three seconds. If an incorrect value was entered 
 * the error is printed to the Serial monitor and explained.
 * 
 * For a complete list of compatable pins for a given microcontroller, see the README file.
 * 
 * @date 2025-08-21
*/

#include "jct_pulseTrainOutput.h"

pulseTrainOutput pto(11);      // Setting the pin that we want to generate pulses from.
uint32_t repeatTimer_ms;       // A timer to repeat generating discrete pulses x milliseconds.

void setup() {
    Serial.begin(9600); // Start Serial for debugging.
    while (!Serial){}   // Wait for the Serial class to be ready.

    Serial.println();

    if(pto.generate(1000, DISCRETE, 4) == true){ // Generates 4 pulses at 1000Hz. The function returns true if generating and false if there was an error.
        Serial.println("No errors. Your chosen pin should be generating.");
    }//
    else{
        Serial.print("Error: ");
        if(pto.getError() == INVALID_PIN){
            Serial.println("There was an invalid pin input in the constructor. Check the allowable pins for the microcontroller you have connected.");
            Serial.println("Allowable pins:");
            Serial.println("    Uno:  D9 and D11.");
            Serial.println("    Mega: D5, D6, D10, D11 and D46.");
        }
        else if(pto.getError() == ZERO_HZ){
            Serial.println("0Hz is not a valid frequency.");
        }
        else if(pto.getError() == ACTIVE){
            Serial.println("The selected pin is currently active.");
        }
        else if(pto.getError() == INVALID_MODE){
            Serial.println("The mode you selected is not available/allowed.");
        }
        else if(pto.getError() == FREQUENCY_HIGH){
            Serial.println("The frequency selected is out of range.");
        }
        while(true){}
    }
    repeatTimer_ms = millis();  // Set the repeat timer. 
}

void loop() {
    if(millis() - repeatTimer_ms > 3000){    // Has the repeat time elapsed.
        pto.generate(1000, DISCRETE, 4);     // Generate another 4 pulses at 1000Hz.
        repeatTimer_ms = millis();           // Reset the timer.
    }
}