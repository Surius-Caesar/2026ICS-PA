#include "cpu/exec.h"

void diff_test_skip_qemu();
void diff_test_skip_nemu();
void raise_intr(uint8_t NO, vaddr_t ret_addr);

make_EHelper(lidt) {
  //Surius: Load IDT register from memory. The operand contains limit (2 bytes) and base (4 bytes).
  uint32_t addr = id_dest->addr;
  cpu.idtr.limit = vaddr_read(addr, 2);
  cpu.idtr.base = vaddr_read(addr + 2, 4);

  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  TODO();

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  TODO();

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(int) {
  //Surius: Trigger software interrupt. Call raise_intr() to handle the exception.
  raise_intr(id_dest->val, decoding.seq_eip);
  decoding.jmp_eip = cpu.eip;
  decoding.is_jmp = 1;

  print_asm("int %s", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(iret) {
  //Surius: Pop EIP, CS, and EFLAGS from stack and restore them.
  rtl_pop(&cpu.eip);
  rtl_pop(&cpu.cs);
  rtl_pop(&cpu.eflags);

  print_asm("iret");
}

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {
  //Surius: Read from I/O port (id_src = port addr) into eAX (id_dest).
  t0 = pio_read(id_src->val, id_dest->width);
  operand_write(id_dest, &t0);

  print_asm_template2(in);

#ifdef DIFF_TEST
  //Surius: NEMU devices differ from QEMU, skip diff check for I/O.
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  //Surius: Write eAX (id_src) to I/O port (id_dest = port addr).
  pio_write(id_dest->val, id_src->width, id_src->val);

  print_asm_template2(out);

#ifdef DIFF_TEST
  //Surius: NEMU devices differ from QEMU, skip diff check for I/O.
  diff_test_skip_qemu();
#endif
}
