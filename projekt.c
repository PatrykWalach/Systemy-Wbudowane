#include <8051.h>
#define TIMER_REFRESH_RATE_TL 18
#define TIMER_REFRESH_RATE_TH 0
#define TIMER_REFRESH_RATE 200
#define TIMER_MODE Tmod_13_BIT
#define timerDisplayDigitsLength 6

enum Tmods {
  Tmod_13_BIT = 0,
  Tmod_16_BIT = 1,
  Tmod_8_BIT = 2,
  Tmod_2_X_8_BIT = 3
};

enum Time { Time_SECONDS = 0, Time_MINUTES = 1, Time_HOURS = 2 };

volatile unsigned char timerDisplayRefreshed;
volatile unsigned char timerDisplayDigits[timerDisplayDigitsLength];
volatile unsigned char timerDisplayEditedSection;
volatile unsigned char isTimerDisplayEdited;

__xdata __at(0x0FF38) volatile unsigned char DISPLAY_DATA;
__xdata __at(0x0FF30) volatile unsigned char DISPLAY_REFRESHED;
__xdata __at(0x0FF22) volatile unsigned char CSKB1;
__xdata __at(0x0FF80) volatile unsigned char LCD_WRITE_COMMAND;
__xdata __at(0x0FF81) volatile unsigned char LCD_WRITE_DATA;
__xdata __at(0x0FF82) volatile unsigned char LCD_READ_COMMAND;
__xdata __at(0x0FF83) volatile unsigned char LCD_READ_DATA;

__code unsigned char timerDisplayDigitToData[] = {
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

void timerDisplayRefresh() {
  P1_6 = 1;

  timerDisplayRefreshed++;
  if (timerDisplayRefreshed >= timerDisplayDigitsLength) {
    timerDisplayRefreshed = 0;
  }

  DISPLAY_REFRESHED = 1 << timerDisplayRefreshed;

  DISPLAY_DATA =
      timerDisplayDigitToData[timerDisplayDigits[timerDisplayRefreshed]] |
      ((timerDisplayRefreshed && timerDisplayRefreshed % 2 == 0) << 7);

  P1_6 = 0;
}

void timerDisplayinit() {
  unsigned char i;
  P1_6 = 1;
  timerDisplayRefreshed = 0;
  DISPLAY_REFRESHED = 1 << timerDisplayRefreshed;

  for (i = 0; i < 6; i++) {
    timerDisplayDigits[i] = 0;
  }

  isTimerDisplayEdited = 0;
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
  timerDisplayEditedSection = 0;
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

  for (i = 0; i < timerDisplayDigitsLength; i++) {
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

  DISPLAY_REFRESHED = 1 << timerDisplayRefreshed;
  P1_6 = 0;
}

unsigned char isMultiplexKeyup(unsigned char key) {
  return (multiplexKeyups & key) == key;
}

enum MatrixKeyboardKey {
  MatrixKeyboardKey_LEFT = 1 << 2,
  MatrixKeyboardKey_RIGHT = 1 << 3,
  MatrixKeyboardKey_UP = 1 << 4,
  MatrixKeyboardKey_DOWN = 1 << 5,
  MatrixKeyboardKey_ESC = 1 << 6,
  MatrixKeyboardKey_ENTER = 1 << 7,
};

unsigned char matrixKeysPressed;
unsigned char previousMatrixKeysPressed;
unsigned char matrixKeyups;

void readMatrixKeyboard() {
  previousMatrixKeysPressed = matrixKeysPressed;
  matrixKeysPressed = ~CSKB1;
  matrixKeyups =
      (previousMatrixKeysPressed ^ matrixKeysPressed) & matrixKeysPressed;
}

unsigned char isMatrixKeyup(unsigned char key) {
  return (matrixKeyups & key) == key;
}

void updateTime() {
  unsigned char i;

  for (i = 0; i < 3; i++) {
    timerDisplayDigits[i * 2 + 1] = time[i] / 10;
    timerDisplayDigits[i * 2] = time[i] % 10;
  }
}

void timerDisplayUpdateSection(unsigned char nextTimerDisplayEditedSection) {
  unsigned char i;

  updateTime();
  timerDisplayEditedSection = nextTimerDisplayEditedSection;

  timerDisplayDigits[timerDisplayEditedSection * 2] = 10;
  timerDisplayDigits[timerDisplayEditedSection * 2 + 1] = 10;

  if (isTimerDisplayEdited) {
    return;
  }

  // prevTime = time;
  for (i = 0; i < 3; i++) {
    prevTime[i] = time[i];
  }
  isTimerDisplayEdited = 1;
}

void addTime(unsigned char hours, unsigned char minutes,
             unsigned char seconds) {
  unsigned char i;

  time[Time_SECONDS] += seconds;
  if (time[Time_SECONDS] >= 60) {
    time[Time_SECONDS] -= 60;
    time[Time_MINUTES]++;
  }
  time[Time_MINUTES] += minutes;
  if (time[Time_MINUTES] >= 60) {
    time[Time_MINUTES] -= 60;
    time[Time_HOURS]++;
  }

  time[Time_HOURS] += hours;
  if (time[Time_HOURS] >= 24) {
    for (i = 0; i < 3; i++) {
      time[i] = 0;
    }
  }

  updateTime();
}

void subtractTime(unsigned char hours, unsigned char minutes,
                  unsigned char seconds) {
  if (seconds <= time[Time_SECONDS]) {
    time[Time_SECONDS] -= seconds;
  }
  if (minutes <= time[Time_MINUTES]) {
    time[Time_MINUTES] -= minutes;
  }
  if (hours <= time[Time_HOURS]) {
    time[Time_HOURS] -= hours;
  }

  updateTime();
}

void handleMultiplexKeyboard() {
  unsigned char i;

  if (isMultiplexKeyup(MultiplexKeyboardKey_LEFT)) {
    timerDisplayUpdateSection(min(timerDisplayEditedSection + 1, 2));
  }
  if (isMultiplexKeyup(MultiplexKeyboardKey_RIGHT)) {
    timerDisplayUpdateSection(max(timerDisplayEditedSection, 1) - 1);
  }

  if (!isTimerDisplayEdited) {
    return;
  }

  if (isMultiplexKeyup(MultiplexKeyboardKey_ESC)) {
    for (i = 0; i < 3; i++) {
      time[i] = prevTime[i];
    }
    updateTime();
    isTimerDisplayEdited = 0;
  }
  if (isMultiplexKeyup(MultiplexKeyboardKey_ENTER)) {
    isTimerDisplayEdited = 0;
  }
  if (isMultiplexKeyup(MultiplexKeyboardKey_UP)) {
    if (timerDisplayEditedSection == Time_HOURS) {
      addTime(1, 0, 0);
    } else if (timerDisplayEditedSection == Time_MINUTES) {
      addTime(0, 1, 0);
    } else {
      addTime(0, 0, 1);
    }
  }
  if (isMultiplexKeyup(MultiplexKeyboardKey_DOWN)) {
    if (timerDisplayEditedSection == Time_HOURS) {
      subtractTime(1, 0, 0);
    } else if (timerDisplayEditedSection == Time_MINUTES) {
      subtractTime(0, 1, 0);
    } else {
      subtractTime(0, 0, 1);
    }
  }
}

void LCDwaitWhileBusy() {
  while ((LCD_READ_COMMAND & (1 << 7)) != 0) {
  }
}

void LCDcommand(unsigned char command) {
  LCDwaitWhileBusy();
  LCD_WRITE_COMMAND = command;
}

#define historyLength 7
__xdata __at(0x4000) volatile char history[historyLength][16];

unsigned char historyIndex;
unsigned char historyWriteIndex;

void writeLCD(unsigned char write) {
  LCDwaitWhileBusy();
  LCD_WRITE_DATA = write;
}

void setHistoryIndex(unsigned char nextHistoryIndex) {
  unsigned char i;
  historyIndex = nextHistoryIndex;
  LCDcommand(0b10000000);
  for (i = 0; i < 16; i++) {
    writeLCD(history[historyIndex][i]);
  }
  LCDcommand(0b11000000);
  for (i = 0; i < 16; i++) {
    writeLCD(history[(historyIndex ? historyIndex : historyLength) - 1][i]);
  }
}

void handleMatrixKeyboard() {
  unsigned char nextHistoryIndex;
  if (isMatrixKeyup(MatrixKeyboardKey_UP)) {
    nextHistoryIndex = (historyIndex + 1) % historyLength;
    if (nextHistoryIndex != historyWriteIndex) {
      setHistoryIndex(nextHistoryIndex);
    }
  }
  if (isMatrixKeyup(MatrixKeyboardKey_DOWN)) {
    nextHistoryIndex = (historyIndex ? historyIndex : historyLength) - 1;
    if (nextHistoryIndex != historyWriteIndex) {
      setHistoryIndex(nextHistoryIndex);
    }
  }
}

void initLCD() {
  LCDcommand(0b111000);
  LCDcommand(0b1111);
  LCDcommand(0b110);
  LCDcommand(0b1);

  historyIndex = 0;
  historyWriteIndex = 0;
}

void historyWrite(char* command) {
  unsigned char i;
  for (i = 0; i < 16; i++) {
    history[historyWriteIndex][i] = command[i];
  }
  setHistoryIndex(historyWriteIndex);
  historyWriteIndex++;
  historyWriteIndex %= historyLength;
}

void main() {
  initTimer0();
  timerDisplayinit();
  initMultiplexKeyboard();
  initLCD();

  while (1) {
    readMultiplexKeyboard();
    handleMultiplexKeyboard();

    readMatrixKeyboard();
    handleMatrixKeyboard();

    if (hasSecondPassed) {
      hasSecondPassed = 0;
      P1_5 = !P1_5;
      counter -= TIMER_REFRESH_RATE;

      if (!isTimerDisplayEdited) {
        addTime(0, 0, 1);
      }
    }
    timerDisplayRefresh();
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

// historyWrite("1             OK");

void handleSerialPortInterrupt(void) __interrupt(4) {}