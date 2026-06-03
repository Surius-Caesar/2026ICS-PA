#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  int key = _read_key();

  if (key != _KEY_NONE) {
    bool keydown = (key & 0x8000) != 0;
    int keycode = key & 0x7fff;
    if (keycode > 0 && keycode < 256) {
      const char *action = keydown ? "kd" : "ku";
      return snprintf(buf, len, "%s %s\n", action, keyname[keycode]);
    }
  }

  return snprintf(buf, len, "t %u\n", (unsigned)_uptime());
}

static char dispinfo[128] __attribute__((used));

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
  _draw_sync();
}

void init_device() {
  _ioe_init();

  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}
