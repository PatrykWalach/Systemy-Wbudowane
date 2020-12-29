#include <8051.h>
#define TIMER_REFRESH_RATE_TL 18
#define TIMER_REFRESH_RATE_TH 0
#define TIMER_REFRESH_RATE 200
#define TIMER_MODE TMOD_13bit
#define digits7SegmentDisplayLength 6

enum TMODs { TMOD_13bit = 0, TMOD_16bit = 1, TMOD_8bit = 2, TMOD_2x8bit = 3 };

volatile unsigned char refreshed7SegmentDisplay;
volatile unsigned char digits7SegmentDisplay[digits7SegmentDisplayLength];
volatile unsigned char edited7SegmentDisplaySection;
volatile unsigned char is7SegmentDisplayEdited;

__xdata __at(0x0FF38) volatile unsigned char DISPLAY_DATA;
__xdata __at(0x0FF30) volatile unsigned char DISPLAY_REFRESHED;

__code unsigned char digitTo7SegmentDisplayData[] = {
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

void refresh7SegmentDisplay() {
  P1_6 = 1;

  refreshed7SegmentDisplay++;
  if (refreshed7SegmentDisplay >= digits7SegmentDisplayLength) {
    refreshed7SegmentDisplay = 0;
  }

  DISPLAY_REFRESHED = 1 << refreshed7SegmentDisplay;

  DISPLAY_DATA = digitTo7SegmentDisplayData
      [digits7SegmentDisplay[refreshed7SegmentDisplay]];

  P1_6 = 0;
}

void init7SegmentDisplay() {
  unsigned char i;
  P1_6 = 1;
  refreshed7SegmentDisplay = 0;
  DISPLAY_REFRESHED = 1 << refreshed7SegmentDisplay;

  for (i = 0; i < 6; i++) {
    digits7SegmentDisplay[i] = 0;
  }

  is7SegmentDisplayEdited = 0;
  P1_6 = 0;
}

volatile unsigned char hasSecondPassed;
volatile unsigned char counter;
unsigned char time[3];

void initTimer0() {
  unsigned char i = 0;
  ET0 = 1;
  EA = 1;
  TMOD = TIMER_MODE;
  TL0 = TIMER_REFRESH_RATE_TL;
  TH0 = TIMER_REFRESH_RATE_TH;
  TR0 = 1;
  hasSecondPassed = 0;
  counter = 0;

  for (i = 0; i < 3; i++) {
    time[i] = 0;
  }
}

unsigned char min(unsigned char a, unsigned char b) { return a < b ? a : b; }
unsigned char max(unsigned char a, unsigned char b) { return a > b ? a : b; }

enum MultiplexKeyboardKey {
  MultiplexKeyboardKey_ENTER = 1 << 0,
  MultiplexKeyboardKey_ESC = 1 << 1,
  MultiplexKeyboardKey_RIGHT = 1 << 2,
  MultiplexKeyboardKey_UP = 1 << 3,
  MultiplexKeyboardKey_DOWN = 1 << 4,
  MultiplexKeyboardKey_LEFT = 1 << 5,
};

unsigned char multiplexKeysPressed;
unsigned char previousMultiplexKeysPressed;
unsigned char multiplexKeyups;
unsigned int prevTime[3];

void initMultiplexKeyboard() {
  unsigned char i = 0;
  edited7SegmentDisplaySection = 0;
  multiplexKeysPressed = 0;

  for (i = 0; i < 3; i++) {
    prevTime[i] = 0;
  }
}

void readMultiplexKeyboard() {
  unsigned char key, i;
  previousMultiplexKeysPressed = multiplexKeysPressed;
  multiplexKeysPressed = 0;
  P1_6 = 1;

  for (i = 0; i < digits7SegmentDisplayLength; i++) {
    key = 1 << i;
    DISPLAY_REFRESHED = key;
    if (P3_5) {
      multiplexKeysPressed |= key;
    } else {
      multiplexKeysPressed &= ~key;
    }
  }

  multiplexKeyups = (previousMultiplexKeysPressed ^ multiplexKeysPressed) &
                    multiplexKeysPressed;

  DISPLAY_REFRESHED = 1 << refreshed7SegmentDisplay;
  P1_6 = 0;
}

unsigned char isMultiplexKeyup(unsigned char key) {
  return (multiplexKeyups & key) == key;
}

void updateTime() {
  unsigned char i;

  for (i = 0; i < 3; i++) {
    digits7SegmentDisplay[i * 2 + 1] = time[i] / 10;
    digits7SegmentDisplay[i * 2] = time[i] % 10;
  }
}

void update7SegmentDisplaySection(
    unsigned char nextEdited7SegmentDisplaySection) {
  updateTime();
  edited7SegmentDisplaySection = nextEdited7SegmentDisplaySection;

  digits7SegmentDisplay[nextEdited7SegmentDisplaySection * 2] = 10;
  digits7SegmentDisplay[nextEdited7SegmentDisplaySection * 2 + 1] = 10;

  // if (is7SegmentDisplayEdited == 1) {
  //   return;
  // }

  // prevTime = time;
  is7SegmentDisplayEdited = 1;
}

void addTime(unsigned char hours, unsigned char minutes,
             unsigned char seconds) {
  unsigned char i;

  time[0] += seconds;
  if (time[0] >= 60) {
    time[0] -= 60;
    time[1]++;
  }
  time[1] += minutes;
  if (time[1] >= 60) {
    time[1] -= 60;
    time[2]++;
  }

  time[2] += hours;
  if (time[2] >= 24) {
    for (i = 0; i < 3; i++) {
      time[i] = 0;
    }
  }

  updateTime();
}

void subtractTime(unsigned char hours, unsigned char minutes,
                  unsigned char seconds) {
  if (seconds <= time[0]) {
    time[0] -= seconds;
  }
  if (minutes <= time[1]) {
    time[1] -= minutes;
  }
  if (hours <= time[2]) {
    time[2] -= hours;
  }

  updateTime();
}

void handleMultiplexKeyboard() {
  unsigned char i;

  if (isMultiplexKeyup(MultiplexKeyboardKey_LEFT)) {
    update7SegmentDisplaySection(min(edited7SegmentDisplaySection + 1, 2));
  }
  if (isMultiplexKeyup(MultiplexKeyboardKey_RIGHT)) {
    update7SegmentDisplaySection(max(edited7SegmentDisplaySection, 1) - 1);
  }

  if (!is7SegmentDisplayEdited) {
    return;
  }

  if (isMultiplexKeyup(MultiplexKeyboardKey_ESC)) {
    for (i = 0; i < 3; i++) {
      time[i] = prevTime[i];
    }
    updateTime();
    is7SegmentDisplayEdited = 0;
  }
  if (isMultiplexKeyup(MultiplexKeyboardKey_ENTER)) {
    is7SegmentDisplayEdited = 0;
  }
  if (isMultiplexKeyup(MultiplexKeyboardKey_UP)) {
    if (edited7SegmentDisplaySection == 2) {
      addTime(1, 0, 0);
    } else if (edited7SegmentDisplaySection == 1) {
      addTime(0, 1, 0);
    } else {
      addTime(0, 0, 1);
    }
  }
  if (isMultiplexKeyup(MultiplexKeyboardKey_DOWN)) {
    if (edited7SegmentDisplaySection == 2) {
      subtractTime(1, 0, 0);
    } else if (edited7SegmentDisplaySection == 1) {
      subtractTime(0, 1, 0);
    } else {
      subtractTime(0, 0, 1);
    }
  }
}

void main() {
  initTimer0();
  init7SegmentDisplay();
  initMultiplexKeyboard();

  while (1) {
    readMultiplexKeyboard();
    handleMultiplexKeyboard();

    if (hasSecondPassed) {
      hasSecondPassed = 0;
      P1_5 = !P1_5;
      counter -= TIMER_REFRESH_RATE;

      if (!is7SegmentDisplayEdited) {
        addTime(0, 0, 1);
      }
    }
    refresh7SegmentDisplay();
  }
}

void handleTimer0Interrupt(void) __interrupt(1) {
  TL0 = TIMER_REFRESH_RATE_TL;
  TH0 = TIMER_REFRESH_RATE_TH;

  counter++;
  if (counter < TIMER_REFRESH_RATE) {
    return;
  }
  hasSecondPassed = 1;
}

void handleSerialPortInterrupt(void) __interrupt(4) {}