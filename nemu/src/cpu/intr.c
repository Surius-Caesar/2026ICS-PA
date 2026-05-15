#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  //Surius: Trigger an interrupt/exception with interrupt number NO.
  //Surius: Use NO to index the IDT and get the gate descriptor.
  
  //Surius: Read gate descriptor from IDT.
  //Surius: IDT is an array of GateDesc, each 8 bytes.
  uint32_t idt_addr = cpu.idtr.base + NO * 8;
  GateDesc *gate = (GateDesc *)guest_to_host(idt_addr);
  
  //Surius: Extract the offset from gate descriptor (16-bit offset fields).
  uint32_t offset = (gate->offset_31_16 << 16) | gate->offset_15_0;
  
  //Surius: Push EFLAGS, CS, and return address (EIP) onto stack.
  //Surius: Stack grows downward, so ESP decreases first.
  cpu.esp -= 4;
  vaddr_write(cpu.esp, 4, cpu.eflags);
  
  cpu.esp -= 4;
  vaddr_write(cpu.esp, 4, cpu.cs);
  
  cpu.esp -= 4;
  vaddr_write(cpu.esp, 4, ret_addr);
  
  //Surius: Jump to the target address specified by the gate descriptor.
  cpu.eip = offset;
}


void dev_raise_intr() {
}
