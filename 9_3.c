
#define ACTIVE 0
#define INACTIVE 1

__sbit __at(0x96) P1_6;
__xdata __at(0x0FF38) unsigned char DISPLAY_DATA;
__xdata __at(0x0FF30) unsigned char DISPLAY_TYPE;
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
unsigned char _indicator6;
unsigned char _indicator5;
unsigned char _indicator4;
unsigned char _indicator3;
unsigned char _indicator2;
unsigned char _indicator1;
unsigned char _active;

void _7_seg_refresh() {
  P1_6 = INACTIVE;
  _active = _active << 1;
  if (_active == 0x80) {
    _active = 0x1;
  }
  DISPLAY_TYPE = _active;

  switch (_active) {
    case 0x1:
      DISPLAY_DATA = _NUMBERS[_indicator1];
      break;
    case 0x2:
      DISPLAY_DATA = _NUMBERS[_indicator2];
      break;
    case 0x4:
      DISPLAY_DATA = _NUMBERS[_indicator3];
      break;
    case 0x8:
      DISPLAY_DATA = _NUMBERS[_indicator4];
      break;
    case 0x10:
      DISPLAY_DATA = _NUMBERS[_indicator5];
      break;
    case 0x20:
      DISPLAY_DATA = _NUMBERS[_indicator6];
      break;
    case 0x40:
      DISPLAY_DATA = _led;
      break;
  }

  P1_6 = ACTIVE;
}

void _7_seg_init() {
  _active = 0x40;
  DISPLAY_TYPE = _active;
  _led = 0x10;
  _indicator6 = 5;
  _indicator5 = 4;
  _indicator4 = 3;
  _indicator3 = 2;
  _indicator2 = 1;
  _indicator1 = 0;
  _7_seg_refresh();
}

unsigned char i;
unsigned char j;

void main() {
  _7_seg_init();

  while (1) {
    for (i = 0; i < 255; i++) {
      for (j = 0; j < 255; j++) {
      }
    }

    _7_seg_refresh();
  }
}
