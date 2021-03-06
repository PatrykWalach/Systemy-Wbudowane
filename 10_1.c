
#define ACTIVE 0
#define INACTIVE 1

__sbit __at(0x96) P1_6;
__sbit __at(0x97) _LED;
__xdata __at(0x0FF38) unsigned char DISPLAY_DATA;
__xdata __at(0x0FF30) unsigned char DISPLAY_TYPE;
__xdata __at(0xFF22) unsigned char CSKB1;

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
unsigned char _active_kbd;

void _7_seg_refresh() {
  P1_6 = INACTIVE;
  _active = _active << 1;
  if (_active == 0x80) {
    _active = 0x1;
  }
  DISPLAY_TYPE = _active;

  switch (_active) {
    case 0x1:
      DISPLAY_DATA = _NUMBERS[_indicators[0]];
      break;
    case 0x2:
      DISPLAY_DATA = _NUMBERS[_indicators[1]];
      break;
    case 0x4:
      DISPLAY_DATA = _NUMBERS[_indicators[2]];
      break;
    case 0x8:
      DISPLAY_DATA = _NUMBERS[_indicators[3]];
      break;
    case 0x10:
      DISPLAY_DATA = _NUMBERS[_indicators[4]];
      break;
    case 0x20:
      DISPLAY_DATA = _NUMBERS[_indicators[5]];
      break;
    case 0x40:
      DISPLAY_DATA = _led;
      break;
  }

  P1_6 = ACTIVE;
}

unsigned char prev;
unsigned char keysUp;
unsigned char pressedKeys;

void _7_seg_init() {
  _active = 0x40;
  _active_kbd = 3;
  DISPLAY_TYPE = _active;
  _led = 0x10;
  _indicators[5] = 5;
  _indicators[4] = 4;
  _indicators[3] = 3;
  _indicators[2] = 2;
  _indicators[1] = 1;
  _indicators[0] = 0;
  keysUp = 0;
  _7_seg_refresh();
}

enum Keys {
  LEFT = 0x4,
  RIGHT = 0x8,
  UP = 0x10,
  DOWN = 0x20,
  ESC = 0x40,
  ENTER = 0x80
};

unsigned char min(unsigned char a, unsigned char b) { return a < b ? a : b; }
unsigned char max(unsigned char a, unsigned char b) { return a > b ? a : b; }

void main() {
  _7_seg_init();

  while (1) {
    pressedKeys = CSKB1;
    keysUp = (prev ^ pressedKeys) & pressedKeys;

    switch (keysUp) {
      case LEFT:
        _active_kbd = min(_active_kbd + 1, 5);
        break;
      case RIGHT:
        _active_kbd = max(_active_kbd, 1) - 1;
        break;
      case UP:
        _indicators[_active_kbd] = min(9, _indicators[_active_kbd] + 1);
        break;
      case DOWN:
        _indicators[_active_kbd] = max(1, _indicators[_active_kbd]) - 1;
        break;
      case ESC:
        _7_seg_init();
        break;
      case ENTER:
        _LED = !_LED;
        break;
    }

    prev = pressedKeys;

    _7_seg_refresh();
  }
}
