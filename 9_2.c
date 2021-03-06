#define ACTIVE 0
#define INACTIVE 1

__sbit __at(0x96) P1_6;

__xdata __at(0x0FF38) unsigned char DISPLAY_DATA;
__xdata __at(0x0FF30) unsigned char DISPLAY_CHOICE;

unsigned char counter;

void _7seg_refresh() {
  P1_6 = INACTIVE;
  DISPLAY_DATA = counter;
  P1_6 = ACTIVE;
}
void _7seg_init() {
  DISPLAY_CHOICE = 0x40;
  counter = 0x10;
  _7seg_refresh();
}

unsigned char i = 0;

void main() {
  _7seg_init();
  while (1) {
    counter = counter << 1;
    if (!counter) {
      counter = 0x1;
    }
    i = 0;
    while (i < 255) {
      i++;
    }
    _7seg_refresh();
  }
}
