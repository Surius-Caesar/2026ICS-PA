#ifndef __ARCH_H__
#define __ARCH_H__

#include <am.h>

#define PMEM_SIZE (128 * 1024 * 1024)
#define PGSIZE    4096    // Bytes mapped by a page

struct _RegSet {
  //Surius: Trap frame layout from stack (low address to high address):
  //Surius: EDI, ESI, EBP, ESP_old, EBX, EDX, ECX, EAX (from pusha)
  //Surius: irq, error_code (from vecsys/vecnull)
  //Surius: EIP, CS, EFLAGS (from hardware)
  uintptr_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  uintptr_t irq, error_code;
  uintptr_t eip, cs, eflags;
};

#define SYSCALL_ARG1(r) ((r)->eax)  //Surius: Syscall number in EAX
#define SYSCALL_ARG2(r) ((r)->ebx)  //Surius: First argument in EBX
#define SYSCALL_ARG3(r) ((r)->ecx)  //Surius: Second argument in ECX
#define SYSCALL_ARG4(r) ((r)->edx)  //Surius: Third argument in EDX

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif
