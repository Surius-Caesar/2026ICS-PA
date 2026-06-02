#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

static char ev_buf[128];
static size_t ev_len = 0;

size_t events_read(void *buf, off_t offset, size_t len) {
  if ((size_t)offset >= ev_len) offset = 0;
  if ((size_t)offset == 0) {
    int key = _read_key();
    if (key != _KEY_NONE) {
      int down = (key & 0x8000) != 0;
      int code = key & ~0x8000;
      if (code > 0 && code < 256 && keyname[code] != NULL) {
        snprintf(ev_buf, sizeof(ev_buf), "k%c %s\n", down ? 'd' : 'u', keyname[code]);
      } else {
        snprintf(ev_buf, sizeof(ev_buf), "t %lu\n", _uptime());
      }
    ev_len = strlen(ev_buf);
    } else {
      snprintf(ev_buf, sizeof(ev_buf), "t %lu\n", _uptime());
    }
    ev_len = strlen(ev_buf);
  }
  if (len == 0) return 0;
  if ((size_t)offset >= ev_len) return 0;
  size_t n = len;
  if ((size_t)offset + n > ev_len) n = ev_len - offset;
  memcpy(buf, ev_buf + offset, n);
  return n;
}

static char dispinfo[128] __attribute__((used));

size_t dispinfo_read(void *buf, off_t offset, size_t len) {
  size_t dlen = strlen(dispinfo);
  if ((size_t)offset >= dlen) return 0;
  size_t n = len;
  if ((size_t)offset + n > dlen) n = dlen - offset;
  memcpy(buf, dispinfo + offset, n);
  return n;
}

size_t fb_write(const void *buf, off_t offset, size_t len) {
  int w = _screen.width;
  int px = offset / 4;
  int x = px % w;
  int y = px / w;
  int pixel_count = len / 4;
  if (pixel_count <= 0) return 0;

  const uint32_t *pixels = buf;
  int written = 0;
  while (written < pixel_count) {
    int n = w - x;
    if (n > pixel_count - written) n = pixel_count - written;
    _draw_rect(pixels + written, x, y, n, 1);
    written += n;
    x = 0;
    y++;
  }
  return len;
}

void init_device() {
  _ioe_init();
  snprintf(dispinfo, sizeof(dispinfo), "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}
