#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

char dispinfo[128] __attribute__((used));

static char ev_buf[128];
static size_t ev_len, ev_offset;
static unsigned long last_timer = (unsigned long)-1;

static void fill_event(void) {
  while (1) {
    int key = _read_key();
    if (key != _KEY_NONE) {
      int keycode = key & 0x7fff;
      bool keydown = (key & 0x8000) != 0;
      if (keycode > 0 && keycode < 256 && keyname[keycode] != NULL) {
        ev_offset = 0;
        snprintf(ev_buf, sizeof(ev_buf), "%s %s\n",
            keydown ? "kd" : "ku", keyname[keycode]);
        ev_len = strlen(ev_buf);
        return;
      }
      /* Bad scancode already read from hardware; skip and try next. */
      continue;
    }

    unsigned t = (unsigned)_uptime();
    if (t != last_timer) {
      ev_offset = 0;
      snprintf(ev_buf, sizeof(ev_buf), "t %u\n", t);
      ev_len = strlen(ev_buf);
      last_timer = t;
      return;
    }
    /* Manual: emit timer only when uptime changes; otherwise wait for key/time. */
  }
}

size_t events_read(void *buf, size_t len) {
  if (len == 0) {
    return 0;
  }

  if (ev_offset >= ev_len) {
    fill_event();
  }

  size_t n = len;
  if (n > ev_len - ev_offset) {
    n = ev_len - ev_offset;
  }
  memcpy(buf, ev_buf + ev_offset, n);
  ev_offset += n;
  return n;
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
  _draw_sync();
}

void init_device() {
  _ioe_init();
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}
