#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
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

size_t events_read(void *buf, size_t len);
size_t dispinfo_read(void *buf, off_t offset, size_t len);
size_t fb_write(const void *buf, off_t offset, size_t len);

static size_t open_offset[NR_FILES];
static bool open_flag[NR_FILES];

void init_fs() {
  file_table[FD_FB].size = _screen.width * _screen.height * 4;
}

size_t fs_filesz(int fd) {
  assert(fd >= 0 && fd < NR_FILES);
  return file_table[fd].size;
}

int fs_open(const char *pathname, int flags, int mode) {
  (void)flags; (void)mode;
  for (int i = 0; i < NR_FILES; i++) {
    if (file_table[i].name && strcmp(file_table[i].name, pathname) == 0) {
      open_offset[i] = 0;
      open_flag[i] = true;
      return i;
    }
  }
  panic("fs_open: file not found: %s", pathname);
  return -1;
}

int fs_read(int fd, void *buf, size_t len) {
  if (fd < 0 || fd >= NR_FILES) return -1;
  if (!open_flag[fd]) return -1;

  switch (fd) {
    case FD_DISPINFO: {
      size_t n = dispinfo_read(buf, open_offset[fd], len);
      open_offset[fd] += n;
      return (int)n;
    }
    case FD_EVENTS: {
      size_t n = events_read(buf, len);
      return (int)n;
    }
    default: {
      Finfo *f = &file_table[fd];
      size_t remain = (f->size > open_offset[fd]) ? (f->size - open_offset[fd]) : 0;
      size_t r = (len < remain) ? len : remain;
      if (r > 0) {
        ramdisk_read(buf, f->disk_offset + open_offset[fd], r);
        open_offset[fd] += r;
      }
      return (int)r;
    }
  }
}

off_t fs_lseek(int fd, off_t offset, int whence) {
  if (fd < 0 || fd >= NR_FILES) return -1;
  Finfo *f = &file_table[fd];
  size_t newoff = 0;
  if (whence == SEEK_SET) {
    newoff = (size_t)offset;
  } else if (whence == SEEK_CUR) {
    newoff = open_offset[fd] + offset;
  } else if (whence == SEEK_END) {
    newoff = f->size + offset;
  } else return -1;
  if (newoff > f->size) return -1;
  open_offset[fd] = newoff;
  return (off_t)open_offset[fd];
}

int fs_close(int fd) {
  if (fd < 0 || fd >= NR_FILES) return -1;
  open_flag[fd] = false;
  open_offset[fd] = 0;
  return 0;
}

int fs_write(int fd, const void *buf, size_t len) {
  if (fd < 0 || fd >= NR_FILES) return -1;
  if (!open_flag[fd]) return -1;

  switch (fd) {
    case FD_FB: {
      size_t w = fb_write(buf, open_offset[fd], len);
      open_offset[fd] += w;
      return (int)w;
    }
    default: {
      Finfo *f = &file_table[fd];
      size_t disk_size = get_ramdisk_size();
      size_t max_writable = (f->disk_offset < disk_size) ? (disk_size - f->disk_offset) : 0;
      size_t remain = (open_offset[fd] < max_writable) ? (max_writable - open_offset[fd]) : 0;
      size_t w = (len < remain) ? len : remain;
      if (w > 0) {
        ramdisk_write(buf, f->disk_offset + open_offset[fd], w);
        open_offset[fd] += w;
        if (open_offset[fd] > f->size) f->size = open_offset[fd];
      }
      return (int)w;
    }
  }
}
