#include "common.h"
#include "fs.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

uintptr_t loader(_Protect *as, const char *filename) {
  // Open the file using filesystem
  int fd = fs_open(filename, 0, 0);
  
  // Get file size
  size_t size = fs_filesz(fd);
  Log("Loader: file=%s, size=%d", filename, size);
  
  // Read file content to DEFAULT_ENTRY
  fs_read(fd, DEFAULT_ENTRY, size);
  
  // Close the file
  fs_close(fd);
  
  Log("Loader: loaded %d bytes to 0x%x", size, DEFAULT_ENTRY);
  return (uintptr_t)DEFAULT_ENTRY;
}
