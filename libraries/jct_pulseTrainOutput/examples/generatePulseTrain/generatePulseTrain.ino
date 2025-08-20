#include "jct_pulseTrainOutput.h"

// Setting up timer1 on pin 9 as a pulse train output.
pulseTrainOutput pto1(9);
// Setting up timer2 on pin 11 as a pulse train output.
pulseTrainOutput pto2(11);

void setup() {
  // Generate 4 discrete pulse at 500Hz from pin 9
  //pto1.generate(100000, DISCRETE, 4);
  // Generate 8 discrete pulse at 1000Hz from pin 11
  pto2.generate(80000, DISCRETE, 4);
}

void loop() {
  // The loop is empty.
  // The hardware is handling both pulse trains independently and continuously.
}