#include "common.h"
#include "syscall.h"
#include "fs.h"

static uintptr_t sys_none(void) {
  return 1;
}

static uintptr_t sys_exit(uintptr_t status) {
  _halt(status);
  return 0;
}

static uintptr_t sys_write(uintptr_t fd, const void *buf, size_t len) {
  if (fd == 1 || fd == 2) {
    const char *p = (const char *)buf;
    for (size_t i = 0; i < len; i++) {
      _putc(p[i]);
    }
    return len;
  }
  int w = fs_write((int)fd, buf, len);
  if (w >= 0) return (uintptr_t)w;
  panic("sys_write: unsupported fd = %d", fd);
  return -1;
}

static uintptr_t sys_brk(uintptr_t brk) {
  (void)brk;
  return 0;
}

static uintptr_t sys_gettimeofday(uintptr_t tv, uintptr_t tz) {
  (void)tz;
  struct timeval *t = (struct timeval *)tv;
  if (t != NULL) {
    unsigned long ms = _uptime();
    t->tv_sec = ms / 1000;
    t->tv_usec = (ms % 1000) * 1000;
  }
  return 0;
}

static uintptr_t sys_open(uintptr_t pathname, uintptr_t flags, uintptr_t mode) {
  const char *p = (const char *)pathname;
  int fd = fs_open(p, (int)flags, (int)mode);
  return (uintptr_t)fd;
}

static uintptr_t sys_read(uintptr_t fd, const void *buf, size_t len) {
  long r = (long)fs_read((int)fd, (void *)buf, len);
  return (uintptr_t)r;
}

static uintptr_t sys_close(uintptr_t fd) {
  return (uintptr_t)fs_close((int)fd);
}

static uintptr_t sys_lseek(uintptr_t fd, uintptr_t offset, uintptr_t whence) {
  off_t r = fs_lseek((int)fd, (off_t)offset, (int)whence);
  return (uintptr_t)r;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  uintptr_t ret = 0;
  switch (a[0]) {
    case SYS_none: ret = sys_none(); break;
    case SYS_exit: ret = sys_exit(a[1]); break;
    case SYS_write: ret = sys_write(a[1], (const void *)a[2], a[3]); break;
    case SYS_open: ret = sys_open(a[1], a[2], a[3]); break;
    case SYS_read: ret = sys_read(a[1], (const void *)a[2], a[3]); break;
    case SYS_close: ret = sys_close(a[1]); break;
    case SYS_lseek: ret = sys_lseek(a[1], a[2], a[3]); break;
    case SYS_brk: ret = sys_brk(a[1]); break;
    case SYS_gettimeofday: ret = sys_gettimeofday(a[1], a[2]); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  SYSCALL_ARG1(r) = ret;
  return NULL;
}
