#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

char dispinfo[128] __attribute__((used));

size_t events_read(void *buf, size_t len) {
  int key = _read_key();

  if (key != _KEY_NONE) {
    int keycode = key & 0x7fff;
    bool keydown = (key & 0x8000) != 0;
    if (keycode > 0 && keycode < 256 && keyname[keycode] != NULL) {
      snprintf(buf, len, "%s %s\n", keydown ? "kd" : "ku", keyname[keycode]);
      return strlen(buf);
    }
  }

  snprintf(buf, len, "t %u\n", (unsigned)_uptime());
  return strlen(buf);
}

void dispinfo_read(void *buf, off_t offset, size_t len) {
  memcpy(buf, dispinfo + offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  int pixel_offset = offset / 4;
  int x = pixel_offset % _screen.width;
  int y = pixel_offset / _screen.width;
  int w = len / 4;
  int h = 1;

  _draw_rect((uint32_t *)buf, x, y, w, h);
}

void init_device() {
  _ioe_init();
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}
