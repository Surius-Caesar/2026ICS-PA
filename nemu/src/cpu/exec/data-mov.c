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
  //Surius: Push all general purpose registers in order: EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI.
  //Surius: When pushing ESP, use the value before PUSHA execution.
  uint32_t esp_at_pusha = cpu.esp;
  rtl_push(&cpu.eax);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ebx);
  rtl_push(&esp_at_pusha);
  rtl_push(&cpu.ebp);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.edi);

  print_asm("pusha");
}

make_EHelper(popa) {
  //Surius: Pop all general purpose registers in reverse order: EDI, ESI, EBP, (skip ESP), EBX, EDX, ECX, EAX.
  rtl_pop(&cpu.edi);
  rtl_pop(&cpu.esi);
  rtl_pop(&cpu.ebp);
  rtl_pop(&t0);  //Surius: Skip ESP
  rtl_pop(&cpu.ebx);
  rtl_pop(&cpu.edx);
  rtl_pop(&cpu.ecx);
  rtl_pop(&cpu.eax);

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
