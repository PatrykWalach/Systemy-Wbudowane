#define ACTIVE 0
#define INACTIVE 1

__sbit __at(0x97) LED;
__sbit __at(0x95) BUZZ;

unsigned char val1, val2;

void test_led_on() { LED = ACTIVE; }
void buzzer_on() { BUZZ = ACTIVE; }

void main() {
  val1 = 30;
  val2 = 31;
  val1 = val1 - val2;

  if (val1) {
    buzzer_on();
  } else {
    test_led_on();
  }

  while (1) {
  }
}
