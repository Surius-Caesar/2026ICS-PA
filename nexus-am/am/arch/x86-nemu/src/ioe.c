#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48   // Note that this is not standard
static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
  //Surius: Read current time from RTC port and subtract boot time to get uptime in ms.
  return inl(RTC_PORT) - boot_time;
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};

extern void* memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  //Surius: Copy pixel rows one by one into framebuffer at the correct (x,y) offset.
  int j;
  for (j = 0; j < h; j++) {
    memcpy(fb + (y + j) * _screen.width + x, pixels + j * w, w * sizeof(uint32_t));
  }
}

void _draw_sync() {
}

#define I8042_STATUS_PORT 0x64
#define I8042_DATA_PORT   0x60
#define I8042_HASKEY_MASK 0x1

int _read_key() {
  //Surius: Poll status register; if a key event is queued, read and return the AM scancode.
  if (inb(I8042_STATUS_PORT) & I8042_HASKEY_MASK) {
    return inl(I8042_DATA_PORT);
  }
  return _KEY_NONE;
}
