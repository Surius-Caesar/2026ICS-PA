#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  //Trigger an interrupt/exception with interrupt number NO.
  //Use NO to index the IDT and get the gate descriptor.
  
  //Read gate descriptor from IDT.
  //IDT is an array of GateDesc, each 8 bytes.
  uint32_t idt_addr = cpu.idtr.base + NO * 8;
  uint32_t low = vaddr_read(idt_addr, 4);
  uint32_t high = vaddr_read(idt_addr + 4, 4);
  
  //Extract the offset from gate descriptor (16-bit offset fields).
  uint32_t offset = ((high >> 16) << 16) | (low & 0xffff);
  
  //Push EFLAGS, CS, and return address (EIP) onto stack.
  //Stack grows downward, so ESP decreases first.
  cpu.esp -= 4;
  vaddr_write(cpu.esp, 4, cpu.eflags);
  
  cpu.esp -= 4;
  vaddr_write(cpu.esp, 4, cpu.cs);
  
  cpu.esp -= 4;
  vaddr_write(cpu.esp, 4, ret_addr);
  
  //Jump to the target address specified by the gate descriptor.
  cpu.eip = offset;
}


void dev_raise_intr() {
}
