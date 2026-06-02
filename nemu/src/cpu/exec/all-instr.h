#include "cpu/exec.h"

//Data move helpers used in PA2 stage2.
make_EHelper(mov);
make_EHelper(movs);
make_EHelper(push);
make_EHelper(pop);
make_EHelper(pusha);
make_EHelper(popa);
make_EHelper(movsx);
make_EHelper(movzx);
make_EHelper(lea);

//Control transfer helpers.
make_EHelper(jmp);
make_EHelper(jcc);
make_EHelper(jmp_rm);
make_EHelper(call);
make_EHelper(call_rm);
make_EHelper(ret);

//Arithmetic helpers for stage2 tests.
make_EHelper(add);
make_EHelper(sub);
make_EHelper(cmp);
make_EHelper(inc);
make_EHelper(dec);
make_EHelper(neg);
make_EHelper(adc);
make_EHelper(sbb);
make_EHelper(mul);
make_EHelper(imul1);
make_EHelper(imul2);
make_EHelper(imul3);
make_EHelper(div);
make_EHelper(idiv);

//Logic helpers for stage2 tests.
make_EHelper(test);
make_EHelper(and);
make_EHelper(or);
make_EHelper(xor);
make_EHelper(not);
make_EHelper(bsr);
make_EHelper(setcc);

//Shift helpers added for stage2 shift tests.
make_EHelper(sar);
make_EHelper(shl);
make_EHelper(shr);

//Data-mov helpers needed for stage2 leave/cltd/cwtl.
make_EHelper(leave);
make_EHelper(cltd);
make_EHelper(cwtl);

//Special helper for one-byte nop.
make_EHelper(nop);

make_EHelper(operand_size);

//PA3 stage-1 system/interrupt helpers.
make_EHelper(lidt);
make_EHelper(int);
make_EHelper(iret);

//IOE port I/O helpers declared for HAS_IOE.
make_EHelper(in);
make_EHelper(out);

//Rotate helpers for MD5/microbench.
make_EHelper(rol);
make_EHelper(ror);

make_EHelper(inv);
make_EHelper(nemu_trap);
