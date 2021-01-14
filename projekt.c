#include <8051.h>
#define TIMER_REFRESH_RATE_TH_0 0b11111101
#define TIMER_REFRESH_RATE_TL_0 0b11000000
#define TIMER_REFRESH_RATE_0 1600
#define TIMER_MODE_0 TimerMode0_16_BIT

enum TimerMode0 {
  TimerMode0_13_BIT = 0,
  TimerMode0_16_BIT = T0_M0,
  TimerMode0_8_BIT = T0_M1,
  TimerMode0_2_X_8_BIT = T0_M0 | T0_M1
};

enum Time { Time_SECONDS = 0, Time_MINUTES = 1, Time_HOURS = 2 };

volatile unsigned char timerDisplayRefreshed;
#define timerDisplayDigitsLength 6
volatile unsigned char timerDisplayDigits[timerDisplayDigitsLength];
volatile unsigned char timerDisplayEditedSection;
__bit isTimerDisplayEdited;

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

volatile unsigned char multiplexKeysPressed[6];
volatile unsigned char previousMultiplexKeysPressed[6];

void initTimerDisplay() {
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
volatile unsigned short counter;
unsigned char time[3];

void initTimer0() {
  unsigned char i = 0;
  ET0 = 1;
  EA = 1;
  TMOD = TMOD & T1_MASK | TIMER_MODE_0;
  TL0 = TIMER_REFRESH_RATE_TL_0;
  TH0 = TIMER_REFRESH_RATE_TH_0;
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
  MultiplexKeyboardKey_ENTER = 0,
  MultiplexKeyboardKey_ESC = 1,
  MultiplexKeyboardKey_RIGHT = 2,
  MultiplexKeyboardKey_UP = 3,
  MultiplexKeyboardKey_DOWN = 4,
  MultiplexKeyboardKey_LEFT = 5,
};

unsigned int prevTime[3];

void initMultiplexKeyboard() {
  unsigned char i = 0;
  timerDisplayEditedSection = 0;

  for (i = 0; i < 6; i++) {
    multiplexKeysPressed[i] = 0;
    previousMultiplexKeysPressed[i] = 0;
  }
  for (i = 0; i < 3; i++) {
    prevTime[i] = 0;
  }
}

unsigned char isMultiplexKeyup(unsigned char key) {
  unsigned char r =
      multiplexKeysPressed[key] &&
      (previousMultiplexKeysPressed[key] != multiplexKeysPressed[key]);

  previousMultiplexKeysPressed[key] = multiplexKeysPressed[key];

  return r;
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

  timerDisplayDigits[timerDisplayEditedSection * 2] = -1;
  timerDisplayDigits[timerDisplayEditedSection * 2 + 1] = -1;

  if (isTimerDisplayEdited) {
    return;
  }

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

volatile unsigned char historyIndex;
volatile unsigned char historyWriteIndex;

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
  unsigned char i, j;
  LCDcommand(0b111000);
  LCDcommand(0b1111);
  LCDcommand(0b110);
  LCDcommand(0b1);

  historyIndex = 0;
  for (i = 0; i < historyLength; i++) {
    for (j = 0; j < 16; j++) {
      history[i][j] = ' ';
    }
  }
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

__bit isSending;
volatile unsigned char receiveBuffer[16];
volatile unsigned char receiveBufferIndex;

volatile unsigned char sendBuffer[11];
volatile unsigned char sendBufferIndex;

enum Commands {
  Commands_GET = 0,
  Commands_EDIT = 1,
  Commands_SET = 2,
  Commands_ERROR,
};

unsigned char prevCommandInterruptedAt;
volatile unsigned char commandInterruptedAt;

void initCommands() {
  unsigned char i = 0;
  ES = 1;
  EA = 1;

  receiveBufferIndex = 0;
  for (i = 0; i < 16; i++) {
    receiveBuffer[i] = ' ';
  }
  sendBufferIndex = 0;
  isSending = 0;
  prevCommandInterruptedAt = 0;
  commandInterruptedAt = 0;

  SCON = 0x50;
  // M0 M1 M2 REN TB8 RB8 TI RI
  TMOD = TMOD & T0_MASK | 0x20;
  TL1 = 0xFA;
  TH1 = 0xFA;
  PCON &= 0x7F;
  TF1 = 0;
  TR1 = 1;
}

void handleTimer0Interrupt(void) __interrupt(TF0_VECTOR) {
  TL0 |= TIMER_REFRESH_RATE_TL_0;
  TH0 = TIMER_REFRESH_RATE_TH_0;

  timerDisplayRefreshed++;

  if (timerDisplayRefreshed >= timerDisplayDigitsLength) {
    timerDisplayRefreshed = 0;
  }

  P1_6 = 1;

  DISPLAY_REFRESHED = 1 << timerDisplayRefreshed;
  multiplexKeysPressed[timerDisplayRefreshed] = P3_5;
  DISPLAY_DATA =
      timerDisplayDigitToData[timerDisplayDigits[timerDisplayRefreshed]] |
      ((timerDisplayRefreshed && timerDisplayRefreshed % 2 == 0) << 7);

  P1_6 = 0;

  counter++;
  if (counter < TIMER_REFRESH_RATE_0) {
    return;
  }
  hasSecondPassed = 1;
}

__code volatile unsigned char tests[3][2][13] = {
    {"GET", "GET"},
    {"EDIT", "EDIT"},
    {"SET 00.00.00", "SET 99.99.99"},
};
__code volatile unsigned char testsLengths[3] = {3, 4, 12};

unsigned char isCommand(unsigned char* string, unsigned char command) {
  unsigned char i;
  for (i = 0; i < testsLengths[command]; i++) {
    if ((string[i] < tests[command][0][i]) ||
        (string[i] > tests[command][1][i])) {
      return 0;
    }
  }
  return 1;
}

void handleSerialPortInterrupt(void) __interrupt(SI0_VECTOR) {
  if (TI) {
    TI = 0;
    isSending = 0;
    return;
  }
  commandInterruptedAt++;
  if (receiveBufferIndex < 12) {
    receiveBuffer[receiveBufferIndex++] = SBUF;
  } else {
    receiveBuffer[10] = '.';
    receiveBuffer[11] = '.';
  }
  RI = 0;
}

void handleCommands() {
  unsigned char i;
  unsigned char ok = 1;
  if (!receiveBufferIndex || prevCommandInterruptedAt != commandInterruptedAt) {
    return;
  }

  if (isCommand(receiveBuffer, Commands_EDIT)) {
    timerDisplayUpdateSection(timerDisplayEditedSection);
  } else if (isCommand(receiveBuffer, Commands_SET)) {
    for (i = 0; i < 3; i++) {
      time[i] = 0;
    }
    addTime((receiveBuffer[4] - '0') * 10 + receiveBuffer[5] - '0',
            (receiveBuffer[7] - '0') * 10 + receiveBuffer[8] - '0',
            (receiveBuffer[10] - '0') * 10 + receiveBuffer[11] - '0');
  } else if (isCommand(receiveBuffer, Commands_GET)) {
    sendBuffer[sendBufferIndex++] = '\n';
    sendBuffer[sendBufferIndex++] = '\r';
    for (i = 0; i < 3; i++) {
      if (i) {
        sendBuffer[sendBufferIndex++] = '.';
      }
      sendBuffer[sendBufferIndex++] = (time[i] % 10) + '0';
      sendBuffer[sendBufferIndex++] = (time[i] / 10) + '0';
    }
  } else {
    ok = 0;
  }

  if (ok) {
    receiveBuffer[13] = ' ';
    receiveBuffer[14] = 'O';
    receiveBuffer[15] = 'K';
  } else {
    receiveBuffer[13] = 'E';
    receiveBuffer[14] = 'R';
    receiveBuffer[15] = 'R';
  }

  historyWrite(receiveBuffer);

  for (i = 0; i < receiveBufferIndex; i++) {
    receiveBuffer[i] = ' ';
  }
  receiveBufferIndex = 0;
}

void handleSend() {
  if (isSending || sendBufferIndex < 1) {
    return;
  }
  isSending = 1;
  SBUF = sendBuffer[--sendBufferIndex];
}

void main() {
  initTimer0();
  initTimerDisplay();
  initMultiplexKeyboard();
  initLCD();
  initCommands();

  while (1) {
    handleMultiplexKeyboard();

    readMatrixKeyboard();
    handleMatrixKeyboard();

    handleCommands();
    handleSend();

    if (!hasSecondPassed) {
      continue;
    }
    hasSecondPassed = 0;
    counter -= TIMER_REFRESH_RATE_0;

    P1_5 = !P1_5;

    prevCommandInterruptedAt = commandInterruptedAt;

    if (!isTimerDisplayEdited) {
      addTime(0, 0, 1);
    }
  }
}
