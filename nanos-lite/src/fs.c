#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum { FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO };

static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin",  0, 0, 0},
  [FD_STDOUT] = {"stdout", 0, 0, 0},
  [FD_STDERR] = {"stderr", 0, 0, 0},
  [FD_FB]     = {"/dev/fb", 0, 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 0, 0, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  file_table[FD_FB].size = _screen.width * _screen.height * 4;
  file_table[FD_DISPINFO].size = strlen(dispinfo);
}

int fs_open(const char *pathname, int flags, int mode) {
  (void)flags;
  (void)mode;

  for (size_t i = 0; i < NR_FILES; i++) {
    if (strcmp(file_table[i].name, pathname) == 0) {
      file_table[i].open_offset = 0;
      return (int)i;
    }
  }
  return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len) {
  if (fd < 0 || (size_t)fd >= NR_FILES) {
    return -1;
  }

  Finfo *f = &file_table[fd];

  switch (fd) {
    case FD_STDIN:
      return 0;
    case FD_STDOUT:
    case FD_STDERR:
      return 0;
    case FD_EVENTS:
      return events_read(buf, len);
    case FD_DISPINFO: {
      size_t remain = f->size - f->open_offset;
      size_t n = len < remain ? len : remain;
      if (n > 0) {
        dispinfo_read(buf, f->open_offset, n);
        f->open_offset += n;
      }
      return (ssize_t)n;
    }
    default: {
      size_t remain = f->size - f->open_offset;
      size_t n = len < remain ? len : remain;
      if (n > 0) {
        ramdisk_read(buf, f->disk_offset + f->open_offset, n);
        f->open_offset += n;
      }
      return (ssize_t)n;
    }
  }
}

ssize_t fs_write(int fd, const void *buf, size_t len) {
  if (fd < 0 || (size_t)fd >= NR_FILES) {
    return -1;
  }

  Finfo *f = &file_table[fd];

  switch (fd) {
    case FD_STDIN:
      return 0;
    case FD_STDOUT:
    case FD_STDERR: {
      const char *p = buf;
      for (size_t i = 0; i < len; i++) {
        _putc(p[i]);
      }
      return (ssize_t)len;
    }
    case FD_FB: {
      size_t remain = f->size - f->open_offset;
      size_t n = len < remain ? len : remain;
      if (n > 0) {
        fb_write(buf, f->open_offset, n);
        f->open_offset += n;
      }
      return (ssize_t)n;
    }
    case FD_EVENTS:
    case FD_DISPINFO:
      return 0;
    default: {
      size_t remain = f->size - f->open_offset;
      size_t n = len < remain ? len : remain;
      if (n > 0) {
        ramdisk_write(buf, f->disk_offset + f->open_offset, n);
        f->open_offset += n;
      }
      return (ssize_t)n;
    }
  }
}

off_t fs_lseek(int fd, off_t offset, int whence) {
  if (fd < 0 || (size_t)fd >= NR_FILES) {
    return -1;
  }

  Finfo *f = &file_table[fd];

  switch (whence) {
    case SEEK_SET:
      f->open_offset = offset;
      break;
    case SEEK_CUR:
      f->open_offset += offset;
      break;
    case SEEK_END:
      f->open_offset = f->size + offset;
      break;
    default:
      return -1;
  }
  return f->open_offset;
}

int fs_close(int fd) {
  if (fd < 0 || (size_t)fd >= NR_FILES) {
    return -1;
  }
  file_table[fd].open_offset = 0;
  return 0;
}

size_t fs_filesz(int fd) {
  if (fd < 0 || (size_t)fd >= NR_FILES) {
    return 0;
  }
  return file_table[fd].size;
}

off_t fs_disk_offset(int fd) {
  if (fd < 0 || (size_t)fd >= NR_FILES) {
    return 0;
  }
  return file_table[fd].disk_offset;
}
