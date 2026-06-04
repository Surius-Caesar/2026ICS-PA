#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48
#define KBD_PORT 0x60

static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
  return inl(RTC_PORT) - boot_time;
}

uint32_t *const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width = 400,
  .height = 300,
};

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      fb[(y + j) * _screen.width + (x + i)] = pixels[j * w + i];
    }
  }
}

void _draw_sync() {
}

int _read_key() {
  if (inl(KBD_PORT + 4) & 0x1) {
    return inl(KBD_PORT);
  }
  return _KEY_NONE;
}
