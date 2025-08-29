/**
 * @file continuousPulses.ino
 * @author CostelloTechnical
 * 
 * @brief This code generates a 1000Hz square wave for 30
 * seconds before it's stopped. If an incorrect value was entered 
 * the error is printed to the Serial monitor and explained.
 * It also checks and prints the current generating state of the 
 * pin to the serial monitor.
 * 
 * For a complete list of compatable pins for a given microcontroller, see the README file.
 * @see https://github.com/CostelloTechnical/pulseTrainOutput/blob/main/README.md
 * 
 * @date 2025-08-21
*/

#include "pulseTrainOutput.h"

pulseTrainOutput pto(11);      // Setting the pin that we want to generate pulses from.
uint32_t stopTimer_ms;         // A timer to stop the continuous wave after x milliseconds.
uint32_t checkGenerating_ms;   // A timer to check if pulses are still being generated. 

void setup() {
    Serial.begin(9600); // Start Serial for debugging.
    while (!Serial){}   // Wait for the Serial class to be ready.

    Serial.println();

    if(pto.generate(1000) == true){ // Generate a continuous 1000Hz square wave. The function returns true if generating and false if there was an error.
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
    }
    stopTimer_ms = millis();       // Set the stop timer.
    checkGenerating_ms = millis(); // Set the check generating timer.    
}

void loop() {
    if(millis() - stopTimer_ms > 30000 && pto.isRunning()){ // Has the stop time elapsed and is the pin generating.
        pto.stop();                                         // Stop the pin generating.
        Serial.println("Stopped generating.");
    }

    if(millis() - checkGenerating_ms > 3000){               // Has the check generating time elapsed.
        Serial.print("Is the pin generating? ");
        Serial.println(pto.isRunning()? "Yes.":"No.");      // If the pin is still generating, print Yes, otherwise print No.
        checkGenerating_ms = millis();                      // Reset the timer.
    }
}