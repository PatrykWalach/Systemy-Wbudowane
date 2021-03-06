
#define ACTIVE 0
#define INACTIVE 1
#include <8051.h>
#define TIMER_25_MS 0x5A

__sbit __at(0x97) IS_LED_NOT_ACTIVE;            // P1.7
__sbit __at(0x96) IS_DISPLAY_ACTIVE;
__xdata __at(0x0FF38) unsigned char DISPLAY_DATA;
__xdata __at(0x0FF30) unsigned char DISPLAY_TYPE;

volatile __sbit secondHasPassed = 0;

__code unsigned char _NUMBERS[] = {
    0x3F,  // ;0
    0x06,  // ;1
    0x5B,  // ;2
    0x4F,  // ;3
    0x66,  // ;4
    0x6D,  // ;5
    0x7D,  // ;6
    0x07,  // ;7
    0x7F,  // ;8
    0x6F,  // ;9
};

unsigned char _led;
unsigned char _indicators[6];
unsigned char _active;
volatile unsigned char counter = 0;


void _7_seg_refresh() {
  IS_DISPLAY_ACTIVE = INACTIVE;
  _active = _active << 1;
  if (_active == 0x80) {
    _active = 0x1;
  }
  DISPLAY_TYPE = _active;

  switch (_active) {
    case 1 << 0:
      DISPLAY_DATA = _NUMBERS[_indicators[0]];
      break;
    case 1 << 1:
      DISPLAY_DATA = _NUMBERS[_indicators[1]];
      break;
    case 1 << 2:
      DISPLAY_DATA = _NUMBERS[_indicators[2]];
      break;
    case 1 << 3:
      DISPLAY_DATA = _NUMBERS[_indicators[3]];
      break;
    case 1 << 4:
      DISPLAY_DATA = _NUMBERS[_indicators[4]];
      break;
    case 1 << 5:
      DISPLAY_DATA = _NUMBERS[_indicators[5]];
      break;
    case 1 << 6:
      DISPLAY_DATA = _led;
      break;
  }

  IS_DISPLAY_ACTIVE = ACTIVE;
}

void _7_seg_init() {
  IS_DISPLAY_ACTIVE = INACTIVE;
  _active = 0x40;
  DISPLAY_TYPE = _active;
  _led = 0x10;
  _indicators[5] = 0;
  _indicators[4] = 0;
  _indicators[3] = 0;
  _indicators[2] = 10;
  _indicators[1] = 0;
  _indicators[0] = 0;
  IS_DISPLAY_ACTIVE = ACTIVE;
  _7_seg_refresh();
}

void init_timer_0() {
  ET0 = 1;
  EA = 1;
  TMOD = 0x01;
  TL0 = 0;
  TH0 = TIMER_25_MS;
  TR0 = 1;
}

unsigned char seconds = 0;

void main() {
  seconds = 250;
  counter = 0;
  secondHasPassed = 0;
  init_timer_0();
  _7_seg_init();

  while (1) {
    _7_seg_refresh();


    _indicators[1] = counter / 10 % 10;
    _indicators[0] = counter % 10;
    if (!secondHasPassed) {
      continue;
    }
    secondHasPassed = 0;
    counter -= 40;
    seconds++;

    _indicators[5] = seconds / 100 % 10;
    _indicators[4] = seconds / 10 % 10;
    _indicators[3] = seconds % 10;
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