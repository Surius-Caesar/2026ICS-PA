#include "common.h"
#include "syscall.h"
#include "fs.h"

//Handler for SYS_none syscall - does nothing and returns 1.
static uintptr_t sys_none(void) {
  return 1;
}

//Handler for SYS_exit syscall - halts with exit status.
static uintptr_t sys_exit(uintptr_t status) {
  _halt(status);
  return 0;  //Never reached
}

static uintptr_t sys_write(uintptr_t fd, const void *buf, size_t len) {
  // Log the write syscall with fd, buf, and len for debugging purposes.
  // the Log function.
  // Log("SYS_write(fd=%d, buf=%p, len=%d)", (int)fd, buf, (int)len);

  // int preview_len = (len < 16) ? (int)len : 16;
  // char preview[17];
  // const char *raw = (const char *)buf;
  // for (int i = 0; i < preview_len; i++) {
  //   char ch = raw[i];
  //   preview[i] = (ch >= 32 && ch <= 126) ? ch : '.';
  // }
  // preview[preview_len] = '\0';
  // Log("SYS_write preview=\"%s\"%s", preview, (len > 16) ? "..." : "");

  int w = fs_write((int)fd, buf, len);
  return (uintptr_t)w;
}

static uintptr_t sys_brk(uintptr_t brk) {
  (void)brk;
  return 0;
}

static uintptr_t sys_open(uintptr_t pathname, uintptr_t flags, uintptr_t mode) {
  const char *p = (const char *)pathname;
  int fd = fs_open(p, (int)flags, (int)mode);
  return (uintptr_t)fd;
}

static uintptr_t sys_read(uintptr_t fd, const void *buf, size_t len) {
  // buf is pointer to user buffer; here we assume direct access
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
    //SYS_none: no-op system call
    case SYS_none:
      ret = sys_none();
      break;
    //SYS_exit: halt with status
    case SYS_exit:
      ret = sys_exit(a[1]);
      break;
    case SYS_write:
      ret = sys_write(a[1], (const void *)a[2], a[3]);
      break;
    case SYS_open:
      ret = sys_open(a[1], a[2], a[3]);
      break;
    case SYS_read:
      ret = sys_read(a[1], (const void *)a[2], a[3]);
      break;
    case SYS_close:
      ret = sys_close(a[1]);
      break;
    case SYS_lseek:
      ret = sys_lseek(a[1], a[2], a[3]);
      break;
    case SYS_brk:
      ret = sys_brk(a[1]);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  //Set return value in EAX (SYSCALL_ARG1)
  SYSCALL_ARG1(r) = ret;

  return NULL;
}
