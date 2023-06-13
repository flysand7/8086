
struct DecodeCtx typedef DecodeCtx;
struct DecodeCtx {
    Memory *mem;
    Far_Ptr start_addr;
    Far_Ptr inst_addr;
    int32_t inst_offs;
    Reg_Index seg_override;
    bool rep_prefix;
    bool lock_prefix;
};

static uint8_t inst_decode8(DecodeCtx *ctx) {
    uint8_t result = mem_load8_at_far(ctx->mem, ctx->inst_addr, ctx->inst_offs);
    ctx->inst_offs += 1;
    return result;
}

static uint16_t inst_decode16(DecodeCtx *ctx) {
    uint16_t result = mem_load16_at_far(ctx->mem, ctx->inst_addr, ctx->inst_offs);
    ctx->inst_offs += 2;
    return result;
}

static uint8_t extract_bits(uint8_t byte, uint8_t bits_remaining, uint8_t nbits) {
    uint8_t mask = (1 << nbits) - 1;
    uint8_t offs = bits_remaining - nbits;
    return (byte >> offs) & mask;
}

static Instruction inst_try_decode(Inst_Encoding encoding, DecodeCtx *ctx) {
    ctx->inst_addr = ctx->start_addr;
    ctx->inst_offs = 0;
    Instruction inst = {0};
    inst.seg_override = REG_ZERO;
    uint8_t byte = inst_decode8(ctx);
    uint8_t bits_remaining = 8;
    uint8_t inst_bits[Enc_Bits_Count] = {0};
    bool inst_has[Enc_Bits_Count] = {0};
    // Go through each field in the encoding, parse them
    // verify that the literals match. If the instruction
    // doesn't match the encoding, the .bytes field will be 0
    for(int i = 0; i < array_count(encoding.bits); ++i) {
        uint8_t usage = encoding.bits[i].usage;
        if(encoding.bits[i].usage == BITS_LAST) {
            break;
        }
        uint8_t bits = encoding.bits[i].value;
        uint8_t bits_count = encoding.bits[i].count;
        // Load new bits if we need them
        if(bits_count != 0) {
            if(bits_remaining == 0) {
                bits_remaining = 8;
                byte = inst_decode8(ctx);
            }
            assert(bits_count <= bits_remaining);
        }
        // Check for whether instruction matches literal, or
        // read the bits into the array
        if(usage == BITS_LIT) {
            if(extract_bits(byte, bits_remaining, bits_count) != bits) {
                return inst;
            }
        }
        else {
            if(bits_count != 0) {
                inst_bits[usage] = extract_bits(byte, bits_remaining, bits_count);
            }
            else {
                inst_bits[usage] = bits;
            }
            inst_has[usage] = true;
        }
        bits_remaining -= bits_count;
    }
    // Read the operands and other instruction bytes
    uint8_t mod = inst_bits[BITS_MOD];
    uint8_t reg = inst_bits[BITS_REG];
    uint8_t rm = inst_bits[BITS_RM];
    uint8_t w = inst_bits[BITS_W];
    uint8_t d = inst_bits[BITS_D];
    // Parse disp
    int16_t disp = 0;
    if(inst_has[BITS_MOD]) {
        bool has_disp = (mod == 0x01) || (mod == 0x02) || (mod == 0x00 && rm == 0x06);
        uint8_t disp_bits = mod == 0x01? 8 : 16;
        if(has_disp) {
            if(disp_bits == 16) {
                disp = (int16_t)inst_decode16(ctx);
            }
            else {
                disp = (int16_t)(int8_t)inst_decode8(ctx);
            }
        }
    }
    Inst_Operand *reg_operand = &inst.operands[d? 0 : 1];
    Inst_Operand *mem_operand = &inst.operands[d? 1 : 0];
    reg_operand->type = 0;
    mem_operand->type = 0;
    // Parse reg operand
    if(inst_has[BITS_REG]) {
        inst.noperands += 1;
        Reg_Operand reg_operands[8][2] = {
            {{REG_A, 1, 0}, {REG_A,  2, 0}},
            {{REG_C, 1, 0}, {REG_C,  2, 0}},
            {{REG_D, 1, 0}, {REG_D,  2, 0}},
            {{REG_B, 1, 0}, {REG_B,  2, 0}},
            {{REG_A, 1, 1}, {REG_SP, 2, 0}},
            {{REG_C, 1, 1}, {REG_BP, 2, 0}},
            {{REG_D, 1, 1}, {REG_SI, 2, 0}},
            {{REG_B, 1, 1}, {REG_DI, 2, 0}},
        };
        uint8_t idx = inst_bits[BITS_REG];
        *reg_operand = (Inst_Operand){
            .type = OPERAND_REG,
            .reg = reg_operands[idx][w]
        };
    }
    else if(inst_has[BITS_V] && inst_bits[BITS_V]) {
        *reg_operand = (Inst_Operand) {
            .type = OPERAND_REG,
            .reg = {REG_C, 2, 0},
        };
    }
    // Parse mod operand
    if(inst_has[BITS_RM]) {
        inst.noperands += 1;
        if(inst_bits[BITS_MOD] != 0x03) {
            Reg_Index reg_idx1[8] = {
                REG_B, REG_B,
                REG_BP, REG_BP,
                REG_SI, REG_DI,
                REG_BP, REG_B,
            };
            Reg_Index reg_idx2[8] = {
                REG_SI,
                REG_DI,
                REG_SI,
                REG_DI,
                REG_ZERO,
                REG_ZERO,
                REG_ZERO,
                REG_ZERO,
            };
            uint8_t idx = inst_bits[BITS_RM];
            Reg_Index reg1 = reg_idx1[idx];
            Reg_Index reg2 = reg_idx2[idx];
            if(inst_bits[BITS_MOD] == 0x00 && inst_bits[BITS_RM] == 0x06) {
                reg1 = REG_ZERO;
            }
            *mem_operand = (Inst_Operand){
                .type = OPERAND_MEM,
                .mem = {
                    .reg1 = reg1,
                    .reg2 = reg2,
                    .disp = disp,
                },
            };
        }
        else {
            Reg_Operand reg_operands[8][2] = {
                {{REG_A, 1, 0}, {REG_A,  2, 0}},
                {{REG_C, 1, 0}, {REG_C,  2, 0}},
                {{REG_D, 1, 0}, {REG_D,  2, 0}},
                {{REG_B, 1, 0}, {REG_B,  2, 0}},
                {{REG_A, 1, 1}, {REG_SP, 2, 0}},
                {{REG_C, 1, 1}, {REG_BP, 2, 0}},
                {{REG_D, 1, 1}, {REG_SI, 2, 0}},
                {{REG_B, 1, 1}, {REG_DI, 2, 0}},
            };
            uint8_t idx = inst_bits[BITS_RM];
            bool wide = w || inst_has[BITS_FLAG_RM_ALWAYS_W];
            *mem_operand = (Inst_Operand){
                .type = OPERAND_REG,
                .reg = reg_operands[idx][wide]
            };
        }
    }
    // Parse segment register operand
    if(inst_has[BITS_SR]) {
        inst.noperands += 1;
        Reg_Index sr = inst_bits[BITS_SR] + REG_ES;
        *reg_operand = (Inst_Operand) {
            .type = OPERAND_REG,
            .reg = {sr, 2, 0},
        };
    }
    // Parse addr operand
    if(inst_has[BITS_ADDR_SHORT]) {
        inst.noperands += 1;
        *mem_operand = (Inst_Operand) {
            .type = OPERAND_ADDR,
            .addr = {
                .type = ADDR_SHORT,
                .offs8 = (int8_t)inst_decode8(ctx),
            }
        };
    }
    else if(inst_has[BITS_ADDR_NEAR]) {
        inst.noperands += 1;
        *mem_operand = (Inst_Operand) {
            .type = OPERAND_ADDR,
            .addr = {
                .type = ADDR_NEAR,
                .offs16 = (int16_t)inst_decode16(ctx),
            },
        };
    }
    else if(inst_has[BITS_ADDR_FAR]) {
        uint16_t offs = inst_decode16(ctx);
        uint16_t seg = inst_decode16(ctx);
        inst.noperands += 1;
        *mem_operand = (Inst_Operand) {
            .type = OPERAND_ADDR,
            .addr = {
                .type = ADDR_FAR,
                .offs = offs,
                .seg = seg,
            }
        };
    }
    // Parse imm operands, if any
    Inst_Operand *other_operand = &inst.operands[0];
    if(other_operand->type != 0) {
        other_operand = &inst.operands[1];
    }
    {
        bool has_imm = false;
        uint8_t imm_size;
        uint16_t imm_value;
        if(inst_has[BITS_IMM]) {
            has_imm = true;
            imm_size = w? 2 : 1;
            bool sign_extend = false;
            if(w && inst_bits[BITS_S]) {
                imm_size = 2;
                sign_extend = true;
            }
            if(sign_extend) {
                int8_t value8 = inst_decode8(ctx);
                int16_t extended = (int16_t) value8;
                imm_value = (uint16_t) extended;
            }
            else if(imm_size == 2) {
                imm_value = inst_decode16(ctx);
            }
            else if(imm_size == 1) {
                imm_value = inst_decode8(ctx);
            }
        }
        else if(inst_has[BITS_IMM8]) {
            has_imm = true;
            imm_size = 1;
            imm_value = inst_decode8(ctx);
        }
        else if(inst_has[BITS_IMM16]) {
            has_imm = true;
            imm_size = 2;
            imm_value = inst_decode16(ctx);
        }
        else if(inst_has[BITS_V] && !inst_bits[BITS_V]) {
            has_imm = true;
            imm_size = 1;
            imm_value = 1;
        }
        if(has_imm) {
            inst.noperands += 1;
            *other_operand = (Inst_Operand) {
                .type = OPERAND_IMM,
                .imm = {
                    .value = imm_value,
                    .size = imm_size,
                },
            };
        }
    }
    if(inst_has[BITS_Z]) {
        inst.z_flag = inst_bits[BITS_Z];
    }
    // Set data size and addr size
    inst.addr_size = 2;
    inst.data_size = 2;
    for(int i = 0; i != inst.noperands; ++i) {
        Inst_Operand_Type optype = inst.operands[i].type;
        if(optype == OPERAND_REG) {
            inst.data_size = inst.operands[i].reg.size;
        }
        else if(optype == OPERAND_IMM) {
            inst.data_size = inst.operands[i].imm.size;
        }
    }
    // Set instruction flags from the context
    if(encoding.opcode == OPCODE_rep) {
        ctx->rep_prefix = true;
    }
    else if(encoding.opcode == OPCODE_lock) {
        ctx->lock_prefix = true;
    }
    else if(encoding.opcode == OPCODE_seg) {
        ctx->seg_override = inst_bits[BITS_SR] + REG_ES;
    }
    else {
        inst.printable = true;
        inst.seg_override = ctx->seg_override;
        inst.rep_prefix = ctx->rep_prefix;
        ctx->seg_override = REG_ZERO;
        ctx->rep_prefix = false;
        ctx->lock_prefix = false;
    }
    inst.opcode = encoding.opcode;
    inst.addr = ctx->inst_addr;
    inst.bytes = ctx->inst_offs;
    return inst;
}

static Instruction inst_decode(DecodeCtx *ctx) {
    Instruction result = {0};
    for(int32_t i = 0; i != array_count(g_encodings); ++i) {
        Inst_Encoding encoding = g_encodings[i];
        result = inst_try_decode(encoding, ctx);
        if(result.bytes > 0) {
            return result;
        }
    }
    return result;
}
