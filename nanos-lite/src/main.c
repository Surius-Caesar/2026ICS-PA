#include "common.h"
#define HAS_ASYE
void init_ramdisk(void); void init_device(void); void init_irq(void); void init_fs(void);
uint32_t loader(_Protect *, const char *);
int main() {
  init_ramdisk(); init_device(); init_irq(); init_fs();
  ((void (*)(void))loader(NULL, "/bin/pal"))();
  panic("Should not reach here");
}
