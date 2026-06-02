#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  // Get keyboard event
  int key = _read_key();
  
  char event_buf[64];
  int event_len = 0;
  
  // Check if there's a key event (priority over timer)
  if (key != _KEY_NONE) {
    bool keydown = (key & 0x8000) ? true : false;
    int keycode = key & 0x7fff;
    const char *action = keydown ? "kd" : "ku";
    const char *keyname_str = keyname[keycode];
    event_len = sprintf(event_buf, "%s %s\n", action, keyname_str);
  } else {
    // No key event, return timer event
    unsigned long time_ms = _uptime();
    event_len = sprintf(event_buf, "t %d\n", (int)time_ms);
  }
  
  // Copy event to output buffer (up to len bytes)
  int copy_len = (event_len < (int)len) ? event_len : (int)len;
  memcpy(buf, event_buf, copy_len);
  
  return copy_len;
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
  // Copy from dispinfo string to buf
  memcpy(buf, dispinfo + offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  // Get screen dimensions from global _screen variable
  int width = _screen.width;
  
  // Calculate the starting position on screen from offset
  // offset is in bytes, each pixel is 4 bytes (RGBA)
  int start_pixel = offset / 4;
  int x = start_pixel % width;
  int y = start_pixel / width;
  
  // Calculate how many pixels to write
  int num_pixels = len / 4;
  
  // Draw rectangle on screen using IOE API
  _draw_rect((uint32_t *)buf, x, y, num_pixels, 1);
}

void init_device() {
  _ioe_init();

  // Get screen information from global _screen variable and format dispinfo string
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}
