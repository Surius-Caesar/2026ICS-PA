#include "common.h"
#include "fs.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

uintptr_t loader(_Protect *as, const char *filename) {
  (void)as;
  const char *path = filename ? filename : "/bin/pal";
  int fd = fs_open(path, 0, 0);
  size_t size = fs_filesz(fd);
  assert(size > 0);

  size_t offset = 0;
  while (offset < size) {
    size_t n = fs_read(fd, (void *)((uintptr_t)DEFAULT_ENTRY + offset), size - offset);
    assert(n > 0);
    offset += n;
  }
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}
