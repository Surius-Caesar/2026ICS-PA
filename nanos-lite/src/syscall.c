#include "common.h"
#include "syscall.h"

//Surius: Handler for SYS_none syscall - does nothing and returns 1.
static uintptr_t sys_none(void) {
  return 1;
}

//Surius: Handler for SYS_exit syscall - halts with exit status.
static uintptr_t sys_exit(uintptr_t status) {
  _halt(status);
  return 0;  //Surius: Never reached
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  uintptr_t ret = 0;
  switch (a[0]) {
    //Surius: SYS_none: no-op system call
    case SYS_none:
      ret = sys_none();
      break;
    //Surius: SYS_exit: halt with status
    case SYS_exit:
      ret = sys_exit(a[1]);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  //Surius: Set return value in EAX (SYSCALL_ARG1)
  SYSCALL_ARG1(r) = ret;

  return NULL;
}
