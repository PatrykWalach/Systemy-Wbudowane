#include <8051.h>

__code unsigned char _WELCOME[] = "WITAJ";
__sbit hasReceived;
__sbit hasSent;
volatile unsigned char receiveBuffer[8];
volatile unsigned char receiveBufferIndex;
unsigned char sendBuffer[11];
unsigned char sendBufferIndex;

void calculate() {
  unsigned char i;
  unsigned char j;
  unsigned char a = 0;
  unsigned char b = 0;

  P1_7 = !P1_7;

  for (i = 3, j = 1; i > 0; i--, j *= 10) {
    a += (receiveBuffer[i - 1] - '0') * j;
  }

  for (i = 3, j = 1; i > 0; i--, j *= 10) {
    b += (receiveBuffer[i + 3] - '0') * j;
  }

  if (receiveBuffer[3] == '+') {
    a += b;
  } else if (receiveBuffer[3] == '-') {
    a -= b;
  } else if (receiveBuffer[3] == '*') {
    a *= b;
  } else {
    a /= b;
  }

  //   sendBufferIndex++;
  for (i = 0, j = 1; i < 3; i++, j *= 10) {
    sendBuffer[sendBufferIndex] = a / j % 10 + '0';
    sendBufferIndex++;
  }
  receiveBufferIndex = 0;
}

void main() {
  ES = 1;
  EA = 1;
  hasReceived = 0;
  hasSent = 0;
  receiveBuffer[0] = 0;
  receiveBufferIndex = 0;
  sendBufferIndex = 0;

  SCON = 0x50;
  TMOD &= 0x2F;
  TMOD |= 0x20;
  TL1 = 0xFD;
  TH1 = 0xFD;
  PCON &= 0x7F;
  TF1 = 0;
  TR1 = 1;

  while (1) {
    if (hasReceived) {
      CY = 0;

      if (receiveBufferIndex < 8) {
        hasReceived = 0;
      } else {
        calculate();
      }
    }

    if (!hasSent) {
      continue;
    }
    hasSent = 0;
    if (sendBufferIndex < 1) {
      continue;
    }
    SBUF = sendBuffer[--sendBufferIndex];
  }
}

void handle_interrupt(void) __interrupt(4) {
  if (TI) {
    TI = 0;
    hasSent = 1;
    return;
  }
  receiveBuffer[receiveBufferIndex] = SBUF;
  SBUF = receiveBuffer[receiveBufferIndex];
  receiveBufferIndex++;
  RI = 0;
  hasReceived = 1;
}
