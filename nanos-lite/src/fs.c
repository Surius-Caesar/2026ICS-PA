#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;  // File read/write offset after opening
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

void init_fs() {
  // Initialize /dev/fb size based on screen dimensions
  file_table[FD_FB].size = _screen.width * _screen.height * 4; // 4 bytes per pixel (RGBA)
}

// Simple open/read/close/lseek implementation backed by file_table and ramdisk
static bool open_flag[NR_FILES];

int fs_open(const char *pathname, int flags, int mode) {
  (void)flags; (void)mode;
  for (int i = 0; i < NR_FILES; i++) {
    if (file_table[i].name && strcmp(file_table[i].name, pathname) == 0) {
      file_table[i].open_offset = 0;
      open_flag[i] = true;
      return i; // use index as fd
    }
  }
  // File not found is an error in our simple filesystem
  panic("File not found: %s", pathname);
  return -1;
}

int fs_read(int fd, void *buf, size_t len) {
  if (fd < 0 || fd >= NR_FILES) return -1;
  if (!open_flag[fd]) return -1;
  
  // Handle special files
  if (fd == FD_STDIN || fd == FD_STDOUT || fd == FD_STDERR) {
    return 0; // Ignore operations on stdin/stdout/stderr
  }
  
  // Handle /dev/events
  if (fd == FD_EVENTS) {
    return events_read(buf, len);
  }
  
  // Handle /proc/dispinfo
  if (fd == FD_DISPINFO) {
    Finfo *f = &file_table[fd];
    size_t remain = (f->size > f->open_offset) ? (f->size - f->open_offset) : 0;
    size_t r = (len < remain) ? len : remain;
    if (r > 0) {
      dispinfo_read(buf, f->open_offset, r);
      f->open_offset += r;
    }
    return (int)r;
  }
  
  // Regular file: read from ramdisk
  Finfo *f = &file_table[fd];
  size_t remain = (f->size > f->open_offset) ? (f->size - f->open_offset) : 0;
  size_t r = (len < remain) ? len : remain;
  if (r > 0) {
    ramdisk_read(buf, f->disk_offset + f->open_offset, r);
    f->open_offset += r;
  }
  return (int)r;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
  if (fd < 0 || fd >= NR_FILES) return -1;
  if (!open_flag[fd]) return -1;
  
  // Handle special files
  if (fd == FD_STDIN || fd == FD_STDOUT || fd == FD_STDERR) {
    return 0;
  }
  
  Finfo *f = &file_table[fd];
  size_t newoff = 0;
  if (whence == SEEK_SET) {
    newoff = (size_t)offset;
  } else if (whence == SEEK_CUR) {
    newoff = f->open_offset + offset;
  } else if (whence == SEEK_END) {
    newoff = f->size + offset;
  } else return -1;
  if (newoff > f->size) return -1;
  f->open_offset = newoff;
  return (off_t)f->open_offset;
}

int fs_close(int fd) {
  if (fd < 0 || fd >= NR_FILES) return -1;
  open_flag[fd] = false;
  file_table[fd].open_offset = 0;
  return 0;
}
int fs_write(int fd, const void *buf, size_t len) {
  if (fd < 0 || fd >= NR_FILES) return -1;
  if (!open_flag[fd]) return -1;
  
  // Handle stdout and stderr: output to serial port
  if (fd == FD_STDOUT || fd == FD_STDERR) {
    const char *p = (const char *)buf;
    for (size_t i = 0; i < len; i++) {
      _putc(p[i]);
    }
    return len;
  }
  
  // Ignore stdin
  if (fd == FD_STDIN) {
    return 0;
  }
  
  // Handle /dev/fb: write to frame buffer
  if (fd == FD_FB) {
    static int fb_write_count = 0;
    if (fb_write_count < 5) {
      Log("fs_write to /dev/fb: len=%d, offset=%d", len, file_table[fd].open_offset);
      fb_write_count++;
    }
    Finfo *f = &file_table[fd];
    fb_write(buf, f->open_offset, len);
    f->open_offset += len;
    return (int)len;
  }
  
  // Regular file: write to ramdisk
  Finfo *f = &file_table[fd];
  size_t remain = (f->size > f->open_offset) ? (f->size - f->open_offset) : 0;
  size_t w = (len < remain) ? len : remain;
  if (w > 0) {
    ramdisk_write(buf, f->disk_offset + f->open_offset, w);
    f->open_offset += w;
  }
  return (int)w;
}

// Helper function to get file size
size_t fs_filesz(int fd) {
  if (fd < 0 || fd >= NR_FILES) return 0;
  return file_table[fd].size;
}
