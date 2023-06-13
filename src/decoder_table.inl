
#if !defined(INST_1ST)
    #define INST_1ST(inst, ...)
#endif

#if !defined(INST)
    #define INST(inst, ...)
#endif

#define B(h)  {.usage = BITS_LIT, .value = 0b ## h, .count = sizeof xstr(h) - 1}

#define F_D        {.usage = BITS_D,   .count = 1}
#define F_W        {.usage = BITS_W,   .count = 1}
#define F_S        {.usage = BITS_S,   .count = 1}
#define F_V        {.usage = BITS_V,   .count = 1}
#define F_Z        {.usage = BITS_Z,   .count = 1}
#define F_MOD      {.usage = BITS_MOD, .count = 2}
#define F_REG      {.usage = BITS_REG, .count = 3}
#define F_RM       {.usage = BITS_RM,  .count = 3}
#define F_SR       {.usage = BITS_SR,  .count = 2}

#define V_D        {.usage = BITS_D,   .value = 1}
#define V_W        {.usage = BITS_W,   .value = 1}
#define V_S        {.usage = BITS_S,   .value = 1}
#define V_V        {.usage = BITS_V,   .value = 1}
#define V_MOD(mod) {.usage = BITS_MOD, .value = 0b ## mod}
#define V_REG(reg) {.usage = BITS_REG, .value = reg}
#define V_RM(rm)   {.usage = BITS_RM,  .value = rm}
#define V_SR(sr)   {.usage = BITS_SR,  .value = sr - REG_ES}


#define IMM   {.usage = BITS_IMM}
#define IMM8  {.usage = BITS_IMM8}
#define IMM16 {.usage = BITS_IMM16}

#define ADDR_S { .usage = BITS_ADDR_SHORT }
#define ADDR_N { .usage = BITS_ADDR_NEAR }
#define ADDR_F { .usage = BITS_ADDR_FAR }


#define RM_ALWAYS_W {.usage = BITS_FLAG_RM_ALWAYS_W}

// ---

INST_1ST(mov, B(100010),   F_D, F_W, F_MOD, F_REG, F_RM)
INST    (mov, B(1100011),  F_W, F_MOD, B(000), F_RM, IMM)
INST    (mov, B(1011),     F_W, F_REG, IMM, V_D)
INST    (mov, B(1010000),  F_W, ADDR_N, V_REG(REG_A))
INST    (mov, B(1010001),  F_W, ADDR_N, V_REG(REG_A), V_D)
INST    (mov, B(10001110), F_MOD, B(0), F_SR, F_RM, V_W, V_D)
INST    (mov, B(10001100), F_MOD, B(0), F_SR, F_RM, V_W)

INST_1ST(push, B(11111111), F_MOD, B(110), F_RM, V_W)
INST    (push, B(01010), F_REG, V_D, V_W)
INST    (push, B(000), F_SR, B(110), V_D)

INST_1ST(pop, B(10001111), F_MOD, B(000), F_RM)
INST    (pop, B(01011), F_REG, V_D, V_W)
INST    (pop, B(000), F_SR, B(111), V_D, V_W)

INST_1ST(xchg, B(1000011), F_W, F_MOD, F_REG, F_RM, V_D)
INST    (xchg, B(10010), F_REG, V_MOD(11), V_RM(REG_A), V_W)

INST_1ST(in, B(1110010), F_W, V_REG(REG_A), V_D, IMM8)
INST    (in, B(1110110), F_W, V_MOD(11), V_REG(REG_A), V_RM(REG_D), V_D, RM_ALWAYS_W)

INST_1ST(out, B(1110011), F_W, V_REG(REG_A), IMM8)
INST    (out, B(1110111), F_W, V_MOD(11), V_REG(REG_A), V_RM(REG_D), RM_ALWAYS_W)

INST_1ST(xlat, B(11010111))
INST_1ST(lea,  B(10001101), F_MOD, F_REG, F_RM, V_W, V_D)
INST_1ST(lds, B(11000101), F_MOD, F_REG, F_RM, V_W, V_D)
INST_1ST(les, B(11000100), F_MOD, F_REG, F_RM, V_W, V_D)
INST_1ST(lahf, B(10011111))
INST_1ST(sahf, B(10011110))
INST_1ST(pushf, B(10011100))
INST_1ST(popf, B(10011101))

INST_1ST(add, B(000000), F_D, F_W, F_MOD, F_REG, F_RM)
INST    (add, B(100000), F_S, F_W, F_MOD, B(000), F_RM, IMM)
INST    (add, B(0000010), F_W, IMM, V_REG(REG_A), V_D)

INST_1ST(adc, B(000100), F_D, F_W, F_MOD, F_REG, F_RM)
INST    (adc, B(100000), F_S, F_W, F_MOD, B(010), F_RM, IMM)
INST    (adc, B(0001010), F_W, IMM, V_REG(REG_A), V_D)

INST_1ST(sub, B(001010), F_D, F_W, F_MOD, F_REG, F_RM)
INST    (sub, B(100000), F_S, F_W, F_MOD, B(101), F_RM, IMM)
INST    (sub, B(0010110), F_W, IMM, V_REG(REG_A), V_D)

INST_1ST(sbb, B(000110), F_D, F_W, F_MOD, F_REG, F_RM)
INST    (sbb, B(100000), F_S, F_W, F_MOD, B(010), F_RM, IMM)
INST    (sbb, B(0001110), F_W, IMM, V_REG(REG_A), V_D)

INST_1ST(inc, B(1111111), F_W, F_MOD, B(000), F_RM)
INST    (inc, B(01000), F_REG, V_D)

INST_1ST(dec, B(1111111), F_W, F_MOD, B(001), F_RM)
INST    (dec, B(01001), F_REG, V_D)

INST_1ST(aaa, B(00110111))
INST_1ST(daa, B(00100111))
INST_1ST(neg, B(1111011), F_W, F_MOD, B(011), F_RM)

INST_1ST(cmp, B(001110), F_D, F_W, F_MOD, F_REG, F_RM)
INST    (cmp, B(100000), F_S, F_W, F_MOD, B(111), F_RM, IMM)
INST    (cmp, B(0011110), F_W, IMM, V_REG(REG_A), V_D)

INST_1ST(aas, B(00111111))
INST_1ST(das, B(00101111))
INST_1ST(mul, B(1111011), F_W, F_MOD, B(100), F_RM, V_REG(REG_A), V_D)
INST_1ST(imul, B(1111011), F_W, F_MOD, B(101), F_RM, V_REG(REG_A), V_D)
INST_1ST(aam, B(11010100), IMM8)
INST_1ST(div, B(1111011), F_W, F_MOD, B(110), F_RM)
INST_1ST(idiv, B(1111011), F_W, F_MOD, B(111), F_RM)
INST_1ST(aad, B(11010101), IMM8)
INST_1ST(cbw, B(10011000))
INST_1ST(cwd, B(10011001))

INST_1ST(not, B(1111011), F_W, F_MOD, B(010), F_RM)
INST_1ST(shl, B(110100), F_V, F_W, F_MOD, B(100), F_RM)
INST_1ST(shr, B(110100), F_V, F_W, F_MOD, B(101), F_RM)
INST_1ST(sar, B(110100), F_V, F_W, F_MOD, B(111), F_RM)
INST_1ST(rol, B(110100), F_V, F_W, F_MOD, B(000), F_RM)
INST_1ST(ror, B(110100), F_V, F_W, F_MOD, B(001), F_RM)
INST_1ST(rcl, B(110100), F_V, F_W, F_MOD, B(010), F_RM)
INST_1ST(rcr, B(110100), F_V, F_W, F_MOD, B(011), F_RM)

INST_1ST(and, B(001000), F_D, F_W, F_MOD, F_REG, F_RM)
INST    (and, B(1000000), F_W, F_MOD, B(100), F_RM, IMM)
INST    (and, B(0010010), F_W, IMM, V_REG(REG_A), V_D)

INST_1ST(test, B(000100), F_D, F_W, F_MOD, F_REG, F_RM)
INST    (test, B(1111011), F_W, F_MOD, B(000), F_RM, IMM)
INST    (test, B(1010100), F_W, IMM, V_REG(REG_A), V_D)

INST_1ST(or, B(000010), F_D, F_W, F_MOD, F_REG, F_RM)
INST    (or, B(1000000), F_W, F_MOD, B(001), F_RM, IMM)
INST    (or, B(0000110), F_W, IMM, V_REG(REG_A), V_D)

INST_1ST(xor, B(001100), F_D, F_W, F_MOD, F_REG, F_RM)
INST    (xor, B(0011010), F_W, F_MOD, B(101), F_RM, IMM)
INST    (xor, B(0000110), F_W, IMM, V_REG(REG_A), V_D)

INST_1ST(rep,  B(1111001), F_Z)
INST_1ST(movs, B(1010010), F_W)
INST_1ST(cmps, B(1010011), F_W)
INST_1ST(scas, B(1010111), F_W)
INST_1ST(lods, B(1010110), F_W)
INST_1ST(stos, B(1010101), F_W)

INST_1ST(call, B(11101000), ADDR_N)
INST    (call, B(11111111), F_MOD, B(010), F_RM)
INST_1ST(callf, B(10011010), ADDR_F) 
INST    (callf, B(11111111), F_MOD, B(011), F_RM)

INST_1ST(jmp, B(11101001), ADDR_N)
INST    (jmp, B(11101011), ADDR_S)
INST    (jmp, B(11111111), F_MOD, B(100), F_RM)
INST_1ST(jmpf, B(11101010), ADDR_F)
INST    (jmpf, B(11111111), F_MOD, B(101), F_RM)

INST_1ST(ret, B(11000011))
INST    (ret, B(11000010), IMM16)
INST_1ST(retf, B(11001011))
INST    (retf, B(11001010), IMM16)

INST_1ST(jz, B(01110100), ADDR_S)
INST_1ST(jl, B(01111100), ADDR_S)
INST_1ST(jle, B(01111110), ADDR_S)
INST_1ST(jb, B(01110010), ADDR_S)
INST_1ST(jbe, B(01110110), ADDR_S)
INST_1ST(jp, B(01111010), ADDR_S)
INST_1ST(jo, B(01110000), ADDR_S)
INST_1ST(js, B(01111000), ADDR_S)
INST_1ST(jnz, B(01110101), ADDR_S)
INST_1ST(jge, B(01111101), ADDR_S)
INST_1ST(jg, B(01111111), ADDR_S)
INST_1ST(jae, B(01110011), ADDR_S)
INST_1ST(ja, B(01110111), ADDR_S)
INST_1ST(jnp, B(01111011), ADDR_S)
INST_1ST(jno, B(01110001), ADDR_S)
INST_1ST(jns, B(01111001), ADDR_S)

INST_1ST(loop, B(11100010), ADDR_S)
INST_1ST(loopz, B(11100001), ADDR_S)
INST_1ST(loopnz, B(11100000), ADDR_S)
INST_1ST(jcxz, B(11100011), ADDR_S)

INST_1ST(int, B(11001101), IMM8)
INST_1ST(int3, B(11001100))
INST_1ST(into, B(11001110))
INST_1ST(iret, B(11001111))

INST_1ST(clc, B(11111000))
INST_1ST(cmc, B(11110101))
INST_1ST(stc, B(11111001))
INST_1ST(cld, B(11111100))
INST_1ST(std, B(11111101))
INST_1ST(cli, B(11111010))
INST_1ST(sti, B(11111011))
INST_1ST(hlt, B(11110100))
INST_1ST(wait, B(10011011))
//INST_1ST(esc, B(11011), F_XXX, F_MOD, F_YYY, F_RM)
INST_1ST(lock, B(11110000))
INST_1ST(seg, B(001), F_SR, B(110))

// ---

#undef B
#undef F_B
#undef F_W
#undef F_S
#undef F_V
#undef F_Z
#undef F_MOD
#undef F_REG
#undef F_RM
#undef V_B
#undef V_W
#undef V_S
#undef V_V
#undef V_MOD
#undef V_REG
#undef V_RM

#undef ADDR_S
#undef ADDR_N
#undef ADDR_F
#undef IMM8
#undef IMM16
#undef IMM

#undef RM_ALWAYS_W

#undef INST
#undef INST_1ST
