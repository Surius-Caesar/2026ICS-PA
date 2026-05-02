#include "cpu/exec.h"

make_EHelper(test) {
  //Surius:     Test is and without write back.
  rtl_and(&t2, &id_dest->val, &id_src->val);

  //Surius:     Logic test updates ZF/SF and clears CF/OF.
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);

  print_asm_template2(test);
}

make_EHelper(and) {
  //Surius:     Do and and write result.
  rtl_and(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  //Surius:     Logic op updates ZF/SF and clears CF/OF.
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);

  print_asm_template2(and);
}

make_EHelper(xor) {
  //Surius:     Do xor and write result back.
  rtl_xor(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  //Surius:     xor clears CF/OF, but updates ZF/SF.
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);

  print_asm_template2(xor);
}

make_EHelper(or) {
  //Surius:     Do or and write result.
  rtl_or(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  //Surius:     Logic op updates ZF/SF and clears CF/OF.
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);

  print_asm_template2(or);
}

make_EHelper(rol) {
  //Surius: Rotate left by (src & 0x1f) bits; no flag update needed in NEMU.
  rtl_andi(&t0, &id_src->val, 0x1f);
  uint32_t width_bits = id_dest->width * 8;
  t0 = t0 % width_bits;
  if (t0 != 0) {
    rtl_shl(&t1, &id_dest->val, &t0);
    t2 = width_bits - t0;
    rtl_shr(&t3, &id_dest->val, &t2);
    rtl_or(&t1, &t1, &t3);
    operand_write(id_dest, &t1);
  }
  print_asm_template2(rol);
}

make_EHelper(ror) {
  //Surius: Rotate right by (src & 0x1f) bits; no flag update needed in NEMU.
  rtl_andi(&t0, &id_src->val, 0x1f);
  uint32_t width_bits = id_dest->width * 8;
  t0 = t0 % width_bits;
  if (t0 != 0) {
    rtl_shr(&t1, &id_dest->val, &t0);
    t2 = width_bits - t0;
    rtl_shl(&t3, &id_dest->val, &t2);
    rtl_or(&t1, &t1, &t3);
    operand_write(id_dest, &t1);
  }
  print_asm_template2(ror);
}

make_EHelper(sar) {
  //Surius:     Arithmetic right shift: sign-extend to 32bit first, then sar.
  rtl_sext(&t2, &id_dest->val, id_dest->width);
  rtl_andi(&t0, &id_src->val, 0x1f);
  rtl_sar(&t2, &t2, &t0);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(sar);
}

make_EHelper(shl) {
  //Surius:     Left shift: mask count to 5 bits, shift, update ZF/SF.
  rtl_andi(&t0, &id_src->val, 0x1f);
  rtl_shl(&t2, &id_dest->val, &t0);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shl);
}

make_EHelper(shr) {
  //Surius:     Logical right shift: mask count to 5 bits, shift, update ZF/SF.
  rtl_andi(&t0, &id_src->val, 0x1f);
  rtl_shr(&t2, &id_dest->val, &t0);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  //Surius:     Not only flips bits, flags not changed.
  rtl_not(&id_dest->val);
  operand_write(id_dest, &id_dest->val);

  print_asm_template1(not);
}
