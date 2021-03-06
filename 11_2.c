#define TIMER_25_MS 0x5A
#include <8051.h>

__sbit __at(0x97) IS_LED_NOT_ACTIVE;          // P1.7

volatile unsigned char counter = 0;
volatile __sbit secondHasPassed = 0;

void init_timer_0() {
  ET0 = 1;
  EA = 1;
  TMOD = 0x01;
  TL0 = 0;
  TH0 = TIMER_25_MS;
  TR0 = 1;
}

void main() {
  counter = 0;
  secondHasPassed = 0;
  init_timer_0();

  while (1) {
    if (secondHasPassed == 0) {
      continue;
    }
    secondHasPassed = 0;
    counter -= 40;
    IS_LED_NOT_ACTIVE = !IS_LED_NOT_ACTIVE;
  }
}

void handle_interrupt(void) __interrupt(1) {
  TH0 = TIMER_25_MS;
  counter++;
  if (counter < 40) {
    return;
  }
  secondHasPassed = 1;
}