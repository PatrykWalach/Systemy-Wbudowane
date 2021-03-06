#include <8051.h>

__code unsigned char _WELCOME[] = "WITAJ";
__sbit hasReceived;
__sbit hasSent;
volatile unsigned char receiveBuffer;

unsigned char i;

void main() {
  ES = 1;
  EA = 1;
  hasReceived = 0;
  hasSent = 0;
  receiveBuffer = 0;

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
      hasReceived = 0;
      i = 0;
      hasSent = 1;
      P1_7 = !P1_7;
    }
    if (!hasSent) {
      continue;
    }
    hasSent = 0;
    if (!_WELCOME[i]) {
      continue;
    }
    SBUF = _WELCOME[i];
    i++;
  }
}

void handle_interrupt(void) __interrupt(4) {
  if (TI) {
    TI = 0;
    hasSent = 1;
    return;
  }
  receiveBuffer = SBUF;
  RI = 0;
  hasReceived = 1;
}
