#include <8051.h>

__code unsigned char _WELCOME[] = "WITAJ";

unsigned char i;

void main() {
  SCON = 0x50;
  TMOD &= 0x2F;
  TMOD |= 0x20;
  TL1 = 0xFD;
  TH1 = 0xFD;
  PCON &= 0x7F;
  TF1 = 0;
  TR1 = 1;
  while (1) {
    if (RI) {
      RI = 0;
      i = 0;
      SBUF = _WELCOME[i];
      P1_7 = !P1_7;
    }
    if (!TI) {
      continue;
    }
    TI = 0;
    i++;
    if (!_WELCOME[i]) {
      continue;
    }
    SBUF = _WELCOME[i];
  }
}