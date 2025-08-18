#include "jct_pulseTrainOutput.h"

pulseTrainOutput pto (UNO);

void setup() {
  pto.generate(1000, 7, DISCRETE);
}

void loop() {

}