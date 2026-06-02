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
  // If buffer is empty or fully consumed, try to generate new event
  if (event_buf_pos >= event_buf_len) {
    // Get keyboard event
    int key = _read_key();
    
    // Only generate event if there's a key press/release
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
      event_buf_pos = 0;
    } else {
      // No key event, return 0 to indicate no data available
      return 0;
    }
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
  static int call_count = 0;
  if (call_count < 5) {
    Log("fb_write called: offset=%d, len=%d", offset, len);
    call_count++;
  }
  
  // Get screen dimensions from global _screen variable
  int width = _screen.width;
  
  // Calculate the starting position on screen from offset
  // offset is in bytes, each pixel is 4 bytes (RGBA)
  int start_pixel = offset / 4;
  int x = start_pixel % width;
  int y = start_pixel / width;
  
  // Calculate how many pixels to write
  int num_pixels = len / 4;
  
  // Handle multi-line drawing
  // If the drawing spans multiple lines, we need to draw row by row
  int pixels_in_first_row = width - x; // Pixels that fit in the first row
  
  if (num_pixels <= pixels_in_first_row) {
    // Single row drawing
    _draw_rect((uint32_t *)buf, x, y, num_pixels, 1);
  } else {
    // Multi-row drawing: draw first partial row
    _draw_rect((uint32_t *)buf, x, y, pixels_in_first_row, 1);
    
    // Draw full rows
    int remaining_pixels = num_pixels - pixels_in_first_row;
    int full_rows = remaining_pixels / width;
    int pixels_in_last_row = remaining_pixels % width;
    
    if (full_rows > 0) {
      _draw_rect((uint32_t *)buf + pixels_in_first_row, 0, y + 1, width, full_rows);
    }
    
    // Draw last partial row
    if (pixels_in_last_row > 0) {
      _draw_rect((uint32_t *)buf + num_pixels - pixels_in_last_row, 0, y + 1 + full_rows, pixels_in_last_row, 1);
    }
  }
}

void init_device() {
  _ioe_init();

  // Get screen information from global _screen variable and format dispinfo string
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}
