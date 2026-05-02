#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  //Surius:     Push current operand value to stack.
  rtl_push(&id_dest->val);

  print_asm_template1(push);
}

make_EHelper(pop) {
  //Surius:     Pop one dword from stack into destination.
  rtl_pop(&t0);
  operand_write(id_dest, &t0);

  print_asm_template1(pop);
}

make_EHelper(pusha) {
  TODO();

  print_asm("pusha");
}

make_EHelper(popa) {
  TODO();

  print_asm("popa");
}

make_EHelper(leave) {
  //Surius:     Leave = mov esp,ebp then pop ebp.
  cpu.esp = cpu.ebp;
  rtl_pop(&t0);
  cpu.ebp = t0;

  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    //Surius:     CWD: sign-extend AX into DX:AX.
    rtlreg_t sign = (cpu.eax & 0x8000u) ? 0xffffu : 0u;
    cpu.edx = (cpu.edx & 0xffff0000u) | sign;
  }
  else {
    //Surius:     CDQ: sign-extend EAX into EDX:EAX.
    cpu.edx = ((int32_t)cpu.eax < 0) ? 0xffffffffu : 0u;
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    //Surius:     CBW: sign-extend AL into AX.
    rtl_sext(&t0, &cpu.eax, 1);
    cpu.eax = (cpu.eax & 0xffff0000u) | (t0 & 0xffffu);
  }
  else {
    //Surius:     CWDE: sign-extend AX into EAX.
    rtl_sext(&cpu.eax, &cpu.eax, 2);
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
