
enum {
    BITS_LAST,
    BITS_LIT,
    // ModRM fields
    BITS_MOD,
    BITS_REG,
    BITS_RM,
    BITS_SR,
    // Flags
    BITS_D,
    BITS_W,
    BITS_S,
    BITS_Z,
    BITS_V,
    // Extra Operands
    BITS_IMM,
    BITS_IMM8,
    BITS_IMM16,
    BITS_ADDR_SHORT,
    BITS_ADDR_NEAR,
    BITS_ADDR_FAR,
    // Extra flags
    BITS_FLAG_RM_ALWAYS_W,
    Enc_Bits_Count,
} typedef Enc_Bits_Usage;

struct Enc_Bits typedef Enc_Bits;
struct Enc_Bits {
    Enc_Bits_Usage usage;
    uint8_t value;
    uint8_t count;
};

struct Inst_Encoding typedef Inst_Encoding;
struct Inst_Encoding {
    Inst_Opcode opcode;
    Enc_Bits bits[16];
};

static Inst_Operand reg_operand(Reg_Index reg, uint8_t size, uint8_t offset) {
    Inst_Operand operand;
    operand.type = OPERAND_REG;
    operand.reg.idx = reg;
    operand.reg.size = size;
    operand.reg.offset = offset;
    return operand;
}

Inst_Encoding g_encodings[] = {
    #define INST_1ST(op, ...) {OPCODE_ ## op, { __VA_ARGS__ }},
    #define INST(op, ...)     {OPCODE_ ## op, { __VA_ARGS__ }},
    #include "decoder_table.inl"
};


static char *g_inst_opcode_names[] = {
    #define INST_1ST(op, ...) xstr(op),
    #include "decoder_table.inl"
};
