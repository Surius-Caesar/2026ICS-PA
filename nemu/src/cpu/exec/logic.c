#include "cpu/exec.h"

make_EHelper(test) {
  //Test is and without write back.
  rtl_and(&t2, &id_dest->val, &id_src->val);

  //Logic test updates ZF/SF and clears CF/OF.
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);

  print_asm_template2(test);
}

make_EHelper(and) {
  //Do and and write result.
  rtl_and(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  //Logic op updates ZF/SF and clears CF/OF.
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);

  print_asm_template2(and);
}

make_EHelper(xor) {
  //Do xor and write result back.
  rtl_xor(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  //xor clears CF/OF, but updates ZF/SF.
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);

  print_asm_template2(xor);
}

make_EHelper(or) {
  //Do or and write result.
  rtl_or(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  //Logic op updates ZF/SF and clears CF/OF.
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);

  print_asm_template2(or);
}

make_EHelper(rol) {
  //Rotate left: shift left by n, OR with shift right by (width-n).
  uint32_t n = id_src->val & 0x1f;
  uint32_t wb = id_dest->width * 8;
  n = n % wb;
  if (n != 0) {
    uint32_t v = id_dest->val;
    uint32_t mask = (wb == 32) ? 0xffffffffu : ((1u << wb) - 1);
    v &= mask;
    t2 = (v << n) | (v >> (wb - n));
    t2 &= mask;
    operand_write(id_dest, &t2);
  }
  print_asm_template2(rol);
}

make_EHelper(ror) {
  //Rotate right: shift right by n, OR with shift left by (width-n).
  uint32_t n = id_src->val & 0x1f;
  uint32_t wb = id_dest->width * 8;
  n = n % wb;
  if (n != 0) {
    uint32_t v = id_dest->val;
    uint32_t mask = (wb == 32) ? 0xffffffffu : ((1u << wb) - 1);
    v &= mask;
    t2 = (v >> n) | (v << (wb - n));
    t2 &= mask;
    operand_write(id_dest, &t2);
  }
  print_asm_template2(ror);
}

make_EHelper(sar) {
  //Arithmetic right shift: sign-extend to 32bit first, then sar.
  rtl_sext(&t2, &id_dest->val, id_dest->width);
  rtl_andi(&t0, &id_src->val, 0x1f);
  rtl_sar(&t2, &t2, &t0);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(sar);
}

make_EHelper(shl) {
  //Left shift: mask count to 5 bits, shift, update ZF/SF.
  rtl_andi(&t0, &id_src->val, 0x1f);
  rtl_shl(&t2, &id_dest->val, &t0);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shl);
}

make_EHelper(shr) {
  //Logical right shift: mask count to 5 bits, shift, update ZF/SF.
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

make_EHelper(bsr) {
  uint32_t mask = (id_src->width == 4) ? 0xffffffffu : ((1u << (id_src->width * 8)) - 1);
  uint32_t src = id_src->val & mask;

  if (src == 0) {
    rtl_set_ZF(&t1);
  }
  else {
    uint32_t idx = 0;
    for (uint32_t i = 0; i < (uint32_t)(id_src->width * 8); i++) {
      if (src & (1u << i)) {
        idx = i;
      }
    }
    t0 = idx;
    operand_write(id_dest, &t0);
    rtl_set_ZF(&tzero);
  }

  print_asm_template2(bsr);
}

make_EHelper(not) {
  //Not only flips bits, flags not changed.
  rtl_not(&id_dest->val);
  operand_write(id_dest, &id_dest->val);

  print_asm_template1(not);
}
