
#define ACTIVE 0
#define INACTIVE 1

__sbit __at(0x96) _DISPLAY;
__sbit __at(0x0B5) P3_5;
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
unsigned char i;

void _7_seg_refresh() {
  _DISPLAY = INACTIVE;
  _active = _active << 1;
  if (_active == 0x80) {
    _active = 0x1;
  }
  DISPLAY_TYPE = _active;

  switch (_active) {
    case 1 << 0:
      DISPLAY_DATA = _NUMBERS[_indicators[0]];
      break;
    case 1 << 1:
      DISPLAY_DATA = _NUMBERS[_indicators[1]];
      break;
    case 1 << 2:
      DISPLAY_DATA = _NUMBERS[_indicators[2]];
      break;
    case 1 << 3:
      DISPLAY_DATA = _NUMBERS[_indicators[3]];
      break;
    case 1 << 4:
      DISPLAY_DATA = _NUMBERS[_indicators[4]];
      break;
    case 1 << 5:
      DISPLAY_DATA = _NUMBERS[_indicators[5]];
      break;
    case 1 << 6:
      DISPLAY_DATA = _led;
      break;
  }

  _DISPLAY = ACTIVE;
}

unsigned char prev;
unsigned char prevMulti;
unsigned char keysUp;
unsigned char pressedKeys;

void _7_seg_init() {
  _DISPLAY = INACTIVE;
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
  _DISPLAY = ACTIVE;
  _7_seg_refresh();
}

enum Keys {
  LEFT = 1 << 2,
  RIGHT = 1 << 3,
  UP = 1 << 4,
  DOWN = 1 << 5,
  ESC = 1 << 6,
  ENTER = 1 << 7
};

enum MultiKeys {
  M_ENTER = 1 << 0,
  M_ESC = 1 << 1,
  M_RIGHT = 1 << 2,
  M_UP = 1 << 3,
  M_DOWN = 1 << 4,
  M_LEFT = 1 << 5,
};

unsigned char min(unsigned char a, unsigned char b) { return a < b ? a : b; }
unsigned char max(unsigned char a, unsigned char b) { return a > b ? a : b; }

void _handle_keyboard(unsigned char pressedKeys, unsigned char prev) {
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
}

unsigned char _read_key(unsigned char pressedKeys, unsigned char keyCode,
                        unsigned char shift) {
  DISPLAY_TYPE = keyCode;
  return P3_5 ? pressedKeys & ~shift : pressedKeys | shift;
}

unsigned char _read_multi_keyboard() {
  unsigned char pressedKeys = 0;

  _DISPLAY = INACTIVE;

  pressedKeys = _read_key(pressedKeys, M_ENTER, ENTER);
  pressedKeys = _read_key(pressedKeys, M_ESC, ESC);
  pressedKeys = _read_key(pressedKeys, M_RIGHT, RIGHT);
  pressedKeys = _read_key(pressedKeys, M_UP, UP);
  pressedKeys = _read_key(pressedKeys, M_DOWN, DOWN);
  pressedKeys = _read_key(pressedKeys, M_LEFT, LEFT);

  DISPLAY_TYPE = _active;
  _DISPLAY = ACTIVE;

  return pressedKeys;
}

void main() {
  _7_seg_init();

  while (1) {
    pressedKeys = CSKB1;
    _handle_keyboard(pressedKeys, prev);
    prev = pressedKeys;

    pressedKeys = _read_multi_keyboard();
    _handle_keyboard(pressedKeys, prevMulti);
    prevMulti = pressedKeys;

    _7_seg_refresh();
  }
}