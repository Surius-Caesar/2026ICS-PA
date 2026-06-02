#include "common.h"
#include "fs.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

uintptr_t loader(_Protect *as, const char *filename) {
  (void)as;

  int fd = fs_open(filename, 0, 0);
  if (fd < 0) {
    panic("loader: file not found: %s", filename);
  }

  size_t size = fs_filesz(fd);
  if (size == 0) {
    panic("loader: file %s has zero size", filename);
  }

  int n = fs_read(fd, (void *)DEFAULT_ENTRY, size);
  if (n != (int)size) {
    panic("loader: read %s failed (expect %d, got %d)", filename, (int)size, n);
  }
  fs_close(fd);

  uint8_t *entry = (uint8_t *)DEFAULT_ENTRY;
  Log("loader: %s loaded (%d bytes), entry = %02x %02x %02x %02x %02x %02x %02x %02x",
      filename, n,
      entry[0], entry[1], entry[2], entry[3],
      entry[4], entry[5], entry[6], entry[7]);

  if (entry[0] == 0x7f && entry[1] == 'E' && entry[2] == 'L' && entry[3] == 'F') {
    panic("loader: %s is still an ELF file; run `make update` in nanos-lite", filename);
  }
  if (entry[0] != 0x55 || entry[1] != 0x89) {
    panic("loader: %s has invalid entry (files.h and ramdisk.img out of sync? run `make update`)", filename);
  }

  return (uintptr_t)DEFAULT_ENTRY;
}
