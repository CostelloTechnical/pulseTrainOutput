#include "jct_pulseTrainOutput.h"

pulseTrainOutput pto (UNO);

void setup() {
  pto.generate(1000, 2, DISCRETE);
}

void loop() {

}