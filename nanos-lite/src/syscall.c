#include "common.h"
#include "syscall.h"

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
  Log("SYS_write(fd=%d, buf=%p, len=%d)", (int)fd, buf, (int)len);

  int preview_len = (len < 16) ? (int)len : 16;
  char preview[17];
  const char *raw = (const char *)buf;
  for (int i = 0; i < preview_len; i++) {
    char ch = raw[i];
    preview[i] = (ch >= 32 && ch <= 126) ? ch : '.';
  }
  preview[preview_len] = '\0';
  Log("SYS_write preview=\"%s\"%s", preview, (len > 16) ? "..." : "");

  if (fd == 1 || fd == 2) {
    const char *p = (const char *)buf;
    for (size_t i = 0; i < len; i++) {
      _putc(p[i]);
    }
    return len;
  }

  panic("sys_write: unsupported fd = %d", fd);
  return -1;
}

static uintptr_t sys_brk(uintptr_t brk) {
  (void)brk;
  return 0;
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
    case SYS_brk:
      ret = sys_brk(a[1]);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  //Set return value in EAX (SYSCALL_ARG1)
  SYSCALL_ARG1(r) = ret;

  return NULL;
}
