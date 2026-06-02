#include "common.h"
#include "fs.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

uintptr_t loader(_Protect *as, const char *filename) {
  // Open the file using filesystem
  int fd = fs_open(filename, 0, 0);
  
  // Get file size
  size_t size = fs_filesz(fd);
  
  // Read file content to DEFAULT_ENTRY
  fs_read(fd, DEFAULT_ENTRY, size);
  
  // Close the file
  fs_close(fd);
  
  return (uintptr_t)DEFAULT_ENTRY;
}
