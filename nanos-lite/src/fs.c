#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  file_table[FD_FB].size = _screen.width * _screen.height * 4;
  file_table[FD_FB].open_offset = 0;
}

int fs_open(const char *pathname, int flags, int mode) {
  (void)flags;
  (void)mode;

  for (int i = 0; i < NR_FILES; i++) {
    if (file_table[i].name && strcmp(file_table[i].name, pathname) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  return -1;
}

int fs_read(int fd, void *buf, size_t len) {
  if (fd < 0 || fd >= NR_FILES) return -1;

  Finfo *f = &file_table[fd];

  switch (fd) {
    case FD_STDIN:
    case FD_STDOUT:
    case FD_STDERR:
      return 0;
    case FD_EVENTS:
      return events_read(buf, len);
    case FD_DISPINFO: {
      size_t remain = (f->size > f->open_offset) ? (f->size - f->open_offset) : 0;
      size_t r = (len < remain) ? len : remain;
      if (r > 0) {
        dispinfo_read(buf, f->open_offset, r);
        f->open_offset += r;
      }
      return (int)r;
    }
    default: {
      size_t remain = (f->size > f->open_offset) ? (f->size - f->open_offset) : 0;
      size_t r = (len < remain) ? len : remain;
      if (r > 0) {
        ramdisk_read(buf, f->disk_offset + f->open_offset, r);
        f->open_offset += r;
      }
      return (int)r;
    }
  }
}

int fs_write(int fd, const void *buf, size_t len) {
  if (fd < 0 || fd >= NR_FILES) return -1;

  Finfo *f = &file_table[fd];

  switch (fd) {
    case FD_STDIN:
      return 0;
    case FD_STDOUT:
    case FD_STDERR: {
      const char *p = (const char *)buf;
      for (size_t i = 0; i < len; i++) {
        _putc(p[i]);
      }
      return (int)len;
    }
    case FD_FB: {
      size_t remain = (f->size > f->open_offset) ? (f->size - f->open_offset) : 0;
      size_t w = (len < remain) ? len : remain;
      if (w > 0) {
        fb_write(buf, f->open_offset, w);
        f->open_offset += w;
      }
      return (int)w;
    }
    default: {
      size_t remain = (f->size > f->open_offset) ? (f->size - f->open_offset) : 0;
      size_t w = (len < remain) ? len : remain;
      if (w > 0) {
        ramdisk_write(buf, f->disk_offset + f->open_offset, w);
        f->open_offset += w;
      }
      return (int)w;
    }
  }
}

off_t fs_lseek(int fd, off_t offset, int whence) {
  if (fd < 0 || fd >= NR_FILES) return -1;

  Finfo *f = &file_table[fd];

  switch (whence) {
    case SEEK_SET:
      f->open_offset = offset;
      return f->open_offset;
    case SEEK_CUR:
      f->open_offset += offset;
      return f->open_offset;
    case SEEK_END:
      f->open_offset = f->size + offset;
      return f->open_offset;
    default:
      return -1;
  }
}

int fs_close(int fd) {
  if (fd < 0 || fd >= NR_FILES) return -1;
  file_table[fd].open_offset = 0;
  return 0;
}

size_t fs_filesz(int fd) {
  if (fd < 0 || fd >= NR_FILES) return 0;
  return file_table[fd].size;
}
