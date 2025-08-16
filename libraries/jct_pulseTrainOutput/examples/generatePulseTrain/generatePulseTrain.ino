#include "jct_pulseTrainOutput.h"

pulseTrainOutput pto (TCCR1A, TCCR1B, OCR1AH, OCR1AL, TCNT1H, TCNT1L);

uint32_t timer = millis();

void setup() {
  pinMode (9, OUTPUT);
  pto.generate(1000, 2, DISCRETE);
}

void loop() {

}