#include "cpu/exec.h"

make_EHelper(mov);
//Surius:     Stage-1 stack helpers for dummy.
make_EHelper(push);
make_EHelper(pop);

//Surius:     Stage-1 control-flow helpers for dummy.
make_EHelper(call);
make_EHelper(ret);

//Surius:     Stage-1 arithmetic/logic helpers for dummy.
make_EHelper(sub);
make_EHelper(xor);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);
