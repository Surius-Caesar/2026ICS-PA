#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

// Internal buffer for events device
static char event_buffer[64];
static int event_buf_len = 0;
static int event_buf_pos = 0;

size_t events_read(void *buf, size_t len) {
  // If buffer is empty or fully consumed, generate new event
  if (event_buf_pos >= event_buf_len) {
    // Get keyboard event
    int key = _read_key();
    
    // Check if there's a key event (priority over timer)
    if (key != _KEY_NONE) {
      bool keydown = (key & 0x8000) ? true : false;
      int keycode = key & 0x7fff;
      const char *action = keydown ? "kd" : "ku";
      // Ensure keycode is within bounds
      if (keycode < 0 || keycode >= 256) {
        keycode = 0; // Default to NONE if out of bounds
      }
      const char *keyname_str = keyname[keycode];
      event_buf_len = sprintf(event_buffer, "%s %s\n", action, keyname_str);
    } else {
      // No key event, return timer event with short timestamp
      unsigned long time_ms = _uptime() % 10000; // Keep timestamp short
      event_buf_len = sprintf(event_buffer, "t %lu\n", time_ms);
    }
    
    event_buf_pos = 0; // Reset position
  }
  
  // Copy as much data as possible from internal buffer to output buffer
  int remain = event_buf_len - event_buf_pos;
  int copy_len = (remain < (int)len) ? remain : (int)len;
  memcpy(buf, event_buffer + event_buf_pos, copy_len);
  event_buf_pos += copy_len;
  
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
