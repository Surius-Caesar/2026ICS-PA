#include "cpu/exec.h"

make_EHelper(mov);
//Surius:     Dummy only needs these stack ops for now.
make_EHelper(push);
make_EHelper(pop);

//Surius:     Dummy will hit these control-flow ops first.
make_EHelper(call);
make_EHelper(ret);

//Surius:     Keep only the arithmetic/logic helpers dummy touches.
make_EHelper(sub);
make_EHelper(xor);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);
