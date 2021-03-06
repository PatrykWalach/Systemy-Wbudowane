#include <8051.h>

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
    if (TI) {
      TI = 0;
    }
    if (!RI) {
      continue;
    }
    RI = 0;
    SBUF = 'H';
    P1_7 = !P1_7;
  }
}