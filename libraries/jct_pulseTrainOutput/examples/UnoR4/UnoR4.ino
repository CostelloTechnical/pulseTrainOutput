#include "FspTimer.h"

FspTimer gpt_timer;

int _pin = 5;
uint32_t counter = 5;
bool _enabled;
bool _is_agt;
TimerPWMChannel_t _pwm_channel;
uint8_t timer_channel;

// This is your ISR/callback function.
void my_pwm_callback(timer_callback_args_t *p_args) {
  // We check for the end-of-cycle event, just like before.
  counter--;
  if (counter == 0) {
    gpt_timer.end();
  }
}

bool cfg_pin(int max_index) {
  /* verify index are good */
  if (_pin < 0 || _pin >= max_index) {
    return false;
  }
  /* getting configuration from table */
  auto pin_cgf = getPinCfgs(_pin, PIN_CFG_REQ_PWM);

  /* verify configuration are good */
  if (pin_cgf[0] == 0) {
    return false;
  }

  timer_channel = GET_CHANNEL(pin_cgf[0]);

  _is_agt = IS_PIN_AGT_PWM(pin_cgf[0]);

  _pwm_channel = IS_PWM_ON_A(pin_cgf[0]) ? CHANNEL_A : CHANNEL_B;

  /* actually configuring PIN function */
  R_IOPORT_PinCfg(&g_ioport_ctrl, g_pin_cfg[_pin].pin, (uint32_t)(IOPORT_CFG_PERIPHERAL_PIN | (_is_agt ? IOPORT_PERIPHERAL_AGT : IOPORT_PERIPHERAL_GPT1)));
}

void setup() {
  int max_index = PINS_COUNT;
  cfg_pin(max_index);

  gpt_timer.begin(TIMER_MODE_PWM, _is_agt, timer_channel, 1000, STANDARD_DUTY_CYCLE_PERC, my_pwm_callback);
  //gpt_timer.begin(TIMER_MODE_PWM, _is_agt, timer_channel, STANDARD_PWM_FREQ_HZ, STANDARD_DUTY_CYCLE_PERC);
  gpt_timer.setup_overflow_irq();
  gpt_timer.add_pwm_extended_cfg();
  gpt_timer.enable_pwm_channel(_pwm_channel);
  gpt_timer.open();
  gpt_timer.start();
}

void loop() {
  // put your main code here, to run repeatedly:
}
