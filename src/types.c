
#define array_count(array) ((sizeof (array)) / (sizeof (array)[0]))

#define str(m) #m
#define xstr(m) str(m)

#define kb 1024
#define mb 1024*kb

struct Memory typedef Memory;
struct Memory {
    uint8_t *buffer;
};

struct Far_Ptr typedef Far_Ptr;
struct Far_Ptr {
    uint16_t seg;
    uint16_t offs;
};

#define FLAG_CF (1<<0)
#define FLAG_PF (1<<2)
#define FLAG_AF (1<<4)
#define FLAG_ZF (1<<6)
#define FLAG_SF (1<<7)
#define FLAG_OF (1<<11)

enum {
    // 8 or 16-bit GP regs
    REG_A,
    REG_C,
    REG_D,
    REG_B,
    // 16-bit gp regs
    REG_SP,
    REG_BP,
    REG_SI,
    REG_DI,
    // segs
    REG_ES,
    REG_CS,
    REG_SS,
    REG_DS,
    // ip, flags
    REG_IP,
    REG_FL,
    // implicit zero reg
    REG_ZERO,
} typedef Reg_Index;

enum {
    #define INST_1ST(opcode, ...) OPCODE_ ## opcode,
    #include "decoder_table.inl"
} typedef Inst_Opcode;

enum {
    ADDR_SHORT,
    ADDR_NEAR,
    ADDR_FAR,
} typedef Addr_Type;

enum {
    OPERAND_NONE,
    OPERAND_REG,
    OPERAND_MEM,
    OPERAND_IMM,
    OPERAND_ADDR,
} typedef Inst_Operand_Type;

struct Reg_Operand typedef Reg_Operand;
struct Reg_Operand {
    Reg_Index idx;
    uint32_t size;
    uint32_t offset;
};

struct Mem_Operand typedef Mem_Operand;
struct Mem_Operand {
    Reg_Index reg1;
    Reg_Index reg2;
    int16_t disp;
};

struct Imm_Operand typedef Imm_Operand;
struct Imm_Operand {
    uint16_t value;
    uint8_t size;
};

struct Addr_Operand typedef Addr_Operand;
struct Addr_Operand {
    Addr_Type type;
    uint16_t seg;
    union {
        uint16_t offs;
        int16_t offs16;
        int16_t offs8;
    };
};

struct Inst_Operand typedef Inst_Operand;
struct Inst_Operand {
    Inst_Operand_Type type;
    union {
        Reg_Operand reg;
        Mem_Operand mem;
        Imm_Operand imm;
        Addr_Operand addr;
    };
};

struct Instruction typedef Instruction;
struct Instruction {
    uint32_t bytes;
    Far_Ptr addr;
    Inst_Opcode opcode;
    uint8_t noperands;
    Inst_Operand operands[2];
    uint8_t z_flag;
    uint8_t rep_prefix;
    uint8_t lock_prefix;
    uint8_t printable;
    Reg_Index seg_override;
    uint8_t data_size;
    uint8_t addr_size;
};

