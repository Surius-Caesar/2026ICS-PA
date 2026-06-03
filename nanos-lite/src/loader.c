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

  off_t offset = fs_disk_offset(fd);
  uint8_t head[4];
  ramdisk_read(head, offset, sizeof(head));
  printk("loader: %s at ramdisk offset %d, disk bytes = %02x %02x %02x %02x\n",
      filename, (int)offset, head[0], head[1], head[2], head[3]);

  int n = fs_read(fd, (void *)DEFAULT_ENTRY, size);
  if (n != (int)size) {
    panic("loader: read %s failed (expect %d, got %d)", filename, (int)size, n);
  }
  fs_close(fd);

  uint8_t *entry = (uint8_t *)DEFAULT_ENTRY;
  printk("loader: %s loaded (%d bytes), mem bytes = %02x %02x %02x %02x %02x %02x %02x %02x\n",
      filename, n,
      entry[0], entry[1], entry[2], entry[3],
      entry[4], entry[5], entry[6], entry[7]);

  if (entry[0] == 0x7f && entry[1] == 'E' && entry[2] == 'L' && entry[3] == 'F') {
    panic("loader: %s is still an ELF file; run `make update` in nanos-lite", filename);
  }
  if (entry[0] != 0x55 || entry[1] != 0x89) {
    panic("loader: %s has invalid entry; run `cd nanos-lite && make clean && make update && make`", filename);
  }

  return (uintptr_t)DEFAULT_ENTRY;
}
