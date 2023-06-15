
struct CPU_State typedef CPU_State;
struct CPU_State {
    Memory *memory;
    DecodeCtx decode_ctx;
    uint16_t regs[REG_ZERO];
};

static CPU_State cpu_init(Memory *mem, Far_Ptr start_addr) {
    CPU_State cpu_state = {0};
    cpu_state.memory = mem;
    // Init decoder ctx
    cpu_state.decode_ctx.mem = mem;
    cpu_state.decode_ctx.seg_override = REG_ZERO;
    // Init registers
    cpu_state.regs[REG_CS] = start_addr.seg;
    cpu_state.regs[REG_IP] = start_addr.offs;
    return cpu_state;
}

static uint8_t load_operand_value8(CPU_State *cpu_state, Instruction inst, Inst_Operand op) {
    if(op.type == OPERAND_REG) {
        uint8_t *reg_bytes = (uint8_t *)&cpu_state->regs[op.reg.idx];
        return reg_bytes[op.reg.offset];
    }
    else if(op.type == OPERAND_MEM) {
        uint16_t reg1_value = cpu_state->regs[op.mem.reg1];
        uint16_t reg2_value = cpu_state->regs[op.mem.reg2];
        int16_t disp = op.mem.disp;
        uint16_t offset = reg1_value + reg2_value + disp;
        uint8_t seg = inst.seg_override;
        if(inst.seg_override == REG_ZERO) {
            seg = REG_DS;
        }
        uint16_t seg_value = cpu_state->regs[seg];
        Far_Ptr far_addr;
        far_addr.seg = seg_value;
        far_addr.offs = offset;
        return mem_load8_at_far(cpu_state->memory, far_addr, 0);
    }
    else if(op.type == OPERAND_ADDR) {
        assert(false);
    }
    else if(op.type == OPERAND_IMM) {
        assert(op.imm.size == 8);
        return op.imm.value;
    }
    assert(false);
    return 0;
}

static uint16_t load_operand_value16(CPU_State *cpu_state, Instruction inst, Inst_Operand op) {
    if(op.type == OPERAND_REG) {
        uint16_t *reg_words = (uint16_t *)&cpu_state->regs[op.reg.idx];
        return reg_words[op.reg.offset];
    }
    else if(op.type == OPERAND_MEM) {
        uint16_t reg1_value = cpu_state->regs[op.mem.reg1];
        uint16_t reg2_value = cpu_state->regs[op.mem.reg2];
        int16_t disp = op.mem.disp;
        uint16_t offset = reg1_value + reg2_value + disp;
        uint8_t seg = inst.seg_override;
        if(inst.seg_override == REG_ZERO) {
            seg = REG_DS;
        }
        uint16_t seg_value = cpu_state->regs[seg];
        Far_Ptr far_addr;
        far_addr.seg = seg_value;
        far_addr.offs = offset;
        return mem_load16_at_far(cpu_state->memory, far_addr, 0);
    }
    else if(op.type == OPERAND_ADDR) {
        assert(false);
    }
    else if(op.type == OPERAND_IMM) {
        assert(op.imm.size == 2);
        return op.imm.value;
    }
    assert(false);
    return 0;
}

static void cpu_jmp_to_op(CPU_State *cpu_state, Instruction inst, Inst_Operand op, bool far) {
    if(op.type == OPERAND_ADDR) {
        if(op.addr.type == ADDR_SHORT) {
            cpu_state->regs[REG_IP] += op.addr.offs8;
        }
        else if(op.addr.type == ADDR_NEAR) {
            cpu_state->regs[REG_IP] += op.addr.offs16;
        }
        else if(op.addr.type == ADDR_FAR) {
            cpu_state->regs[REG_CS] = op.addr.seg;
            cpu_state->regs[REG_IP] = op.addr.offs;
        }
    }
    else if(op.type == OPERAND_MEM) {
        uint16_t reg1_value = cpu_state->regs[op.mem.reg1];
        uint16_t reg2_value = cpu_state->regs[op.mem.reg2];
        int16_t disp = op.mem.disp;
        uint16_t offset = reg1_value + reg2_value + disp;
        uint8_t seg_idx = inst.seg_override;
        if(inst.seg_override == REG_ZERO) {
            seg_idx = REG_CS;
        }
        uint16_t seg_value = cpu_state->regs[seg_idx];
        Far_Ptr far_addr;
        far_addr.seg = seg_value;
        far_addr.offs = offset;
        if(far) {
            uint16_t target_offs = mem_load16_at_far(cpu_state->memory, far_addr, 0); 
            uint16_t target_seg = mem_load16_at_far(cpu_state->memory, far_addr, 2);
            cpu_state->regs[REG_CS] = target_seg;
            cpu_state->regs[REG_IP] = target_offs;
        }
        else {
            uint16_t target_offs = mem_load16_at_far(cpu_state->memory, far_addr, 0); 
            cpu_state->regs[REG_IP] = target_offs;
        }
    }
    else {
        assert(false);
    }
}

static void store_operand_value8(CPU_State *cpu_state, Instruction inst, Inst_Operand op, uint8_t value) {
    if(op.type == OPERAND_REG) {
        uint8_t *reg_words = (uint8_t *)&cpu_state->regs[op.reg.idx];
        reg_words[op.reg.offset] = value;
    }
    else if(op.type == OPERAND_MEM) {
        uint16_t reg1_value = cpu_state->regs[op.mem.reg1];
        uint16_t reg2_value = cpu_state->regs[op.mem.reg2];
        int16_t disp = op.mem.disp;
        uint16_t offset = reg1_value + reg2_value + disp;
        uint8_t seg = inst.seg_override;
        if(inst.seg_override == REG_ZERO) {
            seg = REG_DS;
        }
        uint16_t seg_value = cpu_state->regs[seg];
        Far_Ptr far_addr;
        far_addr.seg = seg_value;
        far_addr.offs = offset;
        mem_store8_at_far(cpu_state->memory, far_addr, 0, value);
    }
    else {
        assert(false);
    }
}

static void store_operand_value16(CPU_State *cpu_state, Instruction inst, Inst_Operand op, uint16_t value) {
    if(op.type == OPERAND_REG) {
        uint16_t *reg_words = (uint16_t *)&cpu_state->regs[op.reg.idx];
        reg_words[op.reg.offset] = value;
    }
    else if(op.type == OPERAND_MEM) {
        uint16_t reg1_value = cpu_state->regs[op.mem.reg1];
        uint16_t reg2_value = cpu_state->regs[op.mem.reg2];
        int16_t disp = op.mem.disp;
        uint16_t offset = reg1_value + reg2_value + disp;
        uint8_t seg = inst.seg_override;
        if(inst.seg_override == REG_ZERO) {
            seg = REG_DS;
        }
        uint16_t seg_value = cpu_state->regs[seg];
        Far_Ptr far_addr;
        far_addr.seg = seg_value;
        far_addr.offs = offset;
        mem_store16_at_far(cpu_state->memory, far_addr, 0, value);
    }
    else {
        assert(false);
    }
}

static void set_flags_szp_value8(CPU_State *cpu_state, uint8_t value) {
    cpu_state->regs[REG_FL] &= ~FLAG_ZF;
    cpu_state->regs[REG_FL] &= ~FLAG_SF;
    cpu_state->regs[REG_FL] &= ~FLAG_PF;
    if(value == 0) {
        cpu_state->regs[REG_FL] |= FLAG_ZF;
    }
    if(value & 0x80) {
        cpu_state->regs[REG_FL] |= FLAG_SF;
    }
    if(__builtin_popcount((int)value) % 2 == 0) {
        cpu_state->regs[REG_FL] |= FLAG_PF;
    }
}

static void set_flags_szp_value16(CPU_State *cpu_state, uint16_t value) {
    cpu_state->regs[REG_FL] &= ~FLAG_ZF;
    cpu_state->regs[REG_FL] &= ~FLAG_SF;
    cpu_state->regs[REG_FL] &= ~FLAG_PF;
    if(value == 0) {
        cpu_state->regs[REG_FL] |= FLAG_ZF;
    }
    if(value & 0x8000) {
        cpu_state->regs[REG_FL] |= FLAG_SF;
    }
    if(__builtin_popcount((int)(uint8_t)value) % 2 == 0) {
        cpu_state->regs[REG_FL] |= FLAG_PF;
    }
}

static void set_flag_af_add(CPU_State *cpu_state, uint16_t arg0, uint16_t arg1) {
    arg0 &= 0x0f;
    arg1 &= 0x0f;
    uint16_t res = arg0 + arg1;
    if(res & 0xfff0) {
        cpu_state->regs[REG_FL] |= FLAG_AF;
    }
}

static void set_flag_af_sub(CPU_State *cpu_state, uint16_t arg0, uint16_t arg1) {
    arg0 &= 0x0f;
    arg1 &= 0x0f;
    uint16_t res = arg0 - arg1;
    if(res & 0xfff0) {
        cpu_state->regs[REG_FL] |= FLAG_AF;
    }
}

static void cpu_exec(CPU_State *cpu_state, Instruction *out_inst) {
    // Decode the next instruction
    Instruction inst;
    do {
        cpu_state->decode_ctx.start_addr.seg = cpu_state->regs[REG_CS];
        cpu_state->decode_ctx.start_addr.offs = cpu_state->regs[REG_IP];
        inst = inst_decode(&cpu_state->decode_ctx);
        if(inst.bytes <= 0) {
            fprintf(
                stderr,
                "Failed to decode instruction at %04x:%04x\n",
                cpu_state->decode_ctx.inst_addr.seg,
                cpu_state->decode_ctx.inst_addr.offs);
            exit(1);
        }
        // TODO check for overflow
        cpu_state->regs[REG_IP] += inst.bytes;
    } while(!inst.printable);
    *out_inst = inst;
    // Do it
    if(inst.opcode == OPCODE_mov) {
        assert(inst.noperands == 2);
        assert(inst.operands[0].type != OPERAND_IMM);
        Inst_Operand dst = inst.operands[0];
        Inst_Operand src = inst.operands[1];
        if(inst.data_size == 1) {
            uint8_t value = load_operand_value8(cpu_state, inst, src);
            store_operand_value8(cpu_state, inst, dst, value);
        }
        else if(inst.data_size == 2) {
            uint16_t value = load_operand_value16(cpu_state, inst, src);
            store_operand_value16(cpu_state, inst, dst, value);
        }
    }
    else if(inst.opcode == OPCODE_add) {
        assert(inst.noperands == 2);
        assert(inst.operands[0].type != OPERAND_IMM);
        Inst_Operand dst = inst.operands[0];
        Inst_Operand src = inst.operands[1];
        if(inst.data_size == 1) {
            uint8_t arg0 = load_operand_value8(cpu_state, inst, dst);
            uint8_t arg1 = load_operand_value8(cpu_state, inst, src);
            uint8_t res = arg0 + arg1;
            store_operand_value8(cpu_state, inst, dst, res);
            set_flags_szp_value8(cpu_state, res);
            cpu_state->regs[REG_FL] &= ~FLAG_CF;
            cpu_state->regs[REG_FL] &= ~FLAG_AF;
            if(res < arg0) cpu_state->regs[REG_FL] |= FLAG_CF;
            set_flag_af_add(cpu_state, arg0, arg1);
        }
        else if(inst.data_size == 2) {
            uint16_t arg0 = load_operand_value16(cpu_state, inst, dst);
            uint16_t arg1 = load_operand_value16(cpu_state, inst, src);
            uint16_t res = arg0 + arg1;
            store_operand_value16(cpu_state, inst, dst, res);
            set_flags_szp_value16(cpu_state, res);
            cpu_state->regs[REG_FL] &= ~FLAG_CF;
            cpu_state->regs[REG_FL] &= ~FLAG_AF;
            if(res < arg0) cpu_state->regs[REG_FL] |= FLAG_CF;
            set_flag_af_add(cpu_state, arg0, arg1);            
        }
    }
    else if(inst.opcode == OPCODE_sub || inst.opcode == OPCODE_cmp) {
        assert(inst.noperands == 2);
        assert(inst.operands[0].type != OPERAND_IMM);
        Inst_Operand dst = inst.operands[0];
        Inst_Operand src = inst.operands[1];
        if(inst.data_size == 1) {
            uint8_t arg0 = load_operand_value8(cpu_state, inst, dst);
            uint8_t arg1 = load_operand_value8(cpu_state, inst, src);
            uint8_t res = arg0 - arg1;
            if(inst.opcode == OPCODE_sub) {
                store_operand_value8(cpu_state, inst, dst, res);
            }
            set_flags_szp_value8(cpu_state, res);
            cpu_state->regs[REG_FL] &= ~FLAG_CF;
            cpu_state->regs[REG_FL] &= ~FLAG_AF;
            if(res > arg0) cpu_state->regs[REG_FL] |= FLAG_CF;
            set_flag_af_sub(cpu_state, arg0, arg1);
        }
        else if(inst.data_size == 2) {
            uint16_t arg0 = load_operand_value16(cpu_state, inst, dst);
            uint16_t arg1 = load_operand_value16(cpu_state, inst, src);
            uint16_t res = arg0 - arg1;
            if(inst.opcode == OPCODE_sub) {
                store_operand_value16(cpu_state, inst, dst, res);
            }
            set_flags_szp_value16(cpu_state, res);
            cpu_state->regs[REG_FL] &= ~FLAG_CF;
            cpu_state->regs[REG_FL] &= ~FLAG_AF;
            if(res > arg0) cpu_state->regs[REG_FL] |= FLAG_CF;
            set_flag_af_sub(cpu_state, arg0, arg1);            
        }        
    }
    else if(inst.opcode == OPCODE_mul) {
        assert(inst.noperands == 2);
        assert(inst.operands[0].type != OPERAND_IMM);
        Inst_Operand dst = inst.operands[0];
        Inst_Operand src = inst.operands[1];
        if(inst.data_size == 1) {
            uint16_t arg0 = load_operand_value8(cpu_state, inst, dst);
            uint16_t arg1 = load_operand_value8(cpu_state, inst, src);
            uint16_t res = arg0 * arg1;
            uint8_t res_lo = res & 0xff;
            uint8_t res_hi = res >> 8;
            if(res & 0xff00) {
                cpu_state->regs[REG_FL] |= FLAG_CF | FLAG_OF;
            }
            else {
                cpu_state->regs[REG_FL] &= ~(FLAG_CF | FLAG_OF);
            }
            cpu_state->regs[REG_A] = res;
            
        }
        else if(inst.data_size == 2) {
            uint32_t arg0 = load_operand_value16(cpu_state, inst, dst);
            uint32_t arg1 = load_operand_value16(cpu_state, inst, src);
            uint32_t res = arg0 * arg1;
            uint16_t res_lo = res & 0xffff;
            uint16_t res_hi = res >> 16;
            if(res_hi) {
                cpu_state->regs[REG_FL] |= FLAG_CF | FLAG_OF;
            }
            else {
                cpu_state->regs[REG_FL] &= ~(FLAG_CF | FLAG_OF);
            }
            cpu_state->regs[REG_A] = res_lo;
            cpu_state->regs[REG_D] = res_hi;
        }
    }
    else if(inst.opcode == OPCODE_div) {
        assert(inst.noperands == 1);
        assert(inst.operands[0].type != OPERAND_IMM);
        Inst_Operand op = inst.operands[0];
        if(inst.data_size == 1) {
            uint16_t arg0 = cpu_state->regs[REG_A];
            uint16_t arg1 = load_operand_value8(cpu_state, inst, op);
            uint16_t quo = arg0 / arg1;
            uint16_t rem = arg0 % arg1;
            cpu_state->regs[REG_A] = (rem << 8) | quo;
            
        }
        else if(inst.data_size == 2) {
            uint32_t arg0 = cpu_state->regs[REG_A] | ((uint32_t)cpu_state->regs[REG_D] << 16);
            uint32_t arg1 = load_operand_value16(cpu_state, inst, op);
            uint32_t quo = arg0 / arg1;
            uint32_t rem = arg0 % arg1;
            cpu_state->regs[REG_A] = quo;
            cpu_state->regs[REG_D] = rem;
        }
    }
    else if(inst.opcode == OPCODE_and || inst.opcode == OPCODE_test) {
        assert(inst.noperands == 2);
        assert(inst.operands[0].type != OPERAND_IMM);
        Inst_Operand dst = inst.operands[0];
        Inst_Operand src = inst.operands[1];
        if(inst.data_size == 1) {
            uint8_t arg0 = load_operand_value8(cpu_state, inst, dst);
            uint8_t arg1 = load_operand_value8(cpu_state, inst, src);
            uint8_t res = arg0 & arg1;
            if(inst.opcode == OPCODE_and) {
                store_operand_value8(cpu_state, inst, dst, res);
            }
            set_flags_szp_value8(cpu_state, res);
            cpu_state->regs[REG_FL] &= ~FLAG_CF;
            cpu_state->regs[REG_FL] &= ~FLAG_AF;
        }
        else if(inst.data_size == 2) {
            uint16_t arg0 = load_operand_value16(cpu_state, inst, dst);
            uint16_t arg1 = load_operand_value16(cpu_state, inst, src);
            uint16_t res = arg0 & arg1;
            store_operand_value16(cpu_state, inst, dst, res);
            set_flags_szp_value16(cpu_state, res);
            cpu_state->regs[REG_FL] &= ~FLAG_CF;
            cpu_state->regs[REG_FL] &= ~FLAG_AF;
        }
    }
    else if(inst.opcode == OPCODE_or) {
        assert(inst.noperands == 2);
        assert(inst.operands[0].type != OPERAND_IMM);
        Inst_Operand dst = inst.operands[0];
        Inst_Operand src = inst.operands[1];
        if(inst.data_size == 1) {
            uint8_t arg0 = load_operand_value8(cpu_state, inst, dst);
            uint8_t arg1 = load_operand_value8(cpu_state, inst, src);
            uint8_t res = arg0 | arg1;
            store_operand_value8(cpu_state, inst, dst, res);
            set_flags_szp_value8(cpu_state, res);
            cpu_state->regs[REG_FL] &= ~FLAG_CF;
            cpu_state->regs[REG_FL] &= ~FLAG_AF;
        }
        else if(inst.data_size == 2) {
            uint16_t arg0 = load_operand_value16(cpu_state, inst, dst);
            uint16_t arg1 = load_operand_value16(cpu_state, inst, src);
            uint16_t res = arg0 | arg1;
            store_operand_value16(cpu_state, inst, dst, res);
            set_flags_szp_value16(cpu_state, res);
            cpu_state->regs[REG_FL] &= ~FLAG_CF;
            cpu_state->regs[REG_FL] &= ~FLAG_AF;
        }
    }
    else if(inst.opcode == OPCODE_xor) {
        assert(inst.noperands == 2);
        assert(inst.operands[0].type != OPERAND_IMM);
        Inst_Operand dst = inst.operands[0];
        Inst_Operand src = inst.operands[1];
        if(inst.data_size == 1) {
            uint8_t arg0 = load_operand_value8(cpu_state, inst, dst);
            uint8_t arg1 = load_operand_value8(cpu_state, inst, src);
            uint8_t res = arg0 ^ arg1;
            store_operand_value8(cpu_state, inst, dst, res);
            set_flags_szp_value8(cpu_state, res);
            cpu_state->regs[REG_FL] &= ~FLAG_CF;
            cpu_state->regs[REG_FL] &= ~FLAG_AF;
        }
        else if(inst.data_size == 2) {
            uint16_t arg0 = load_operand_value16(cpu_state, inst, dst);
            uint16_t arg1 = load_operand_value16(cpu_state, inst, src);
            uint16_t res = arg0 ^ arg1;
            store_operand_value16(cpu_state, inst, dst, res);
            set_flags_szp_value16(cpu_state, res);
            cpu_state->regs[REG_FL] &= ~FLAG_CF;
            cpu_state->regs[REG_FL] &= ~FLAG_AF;
        }
    }
    else if(inst.opcode == OPCODE_inc) {
        assert(inst.noperands == 1);
        assert(inst.operands[0].type != OPERAND_IMM);
        Inst_Operand op = inst.operands[0];
        if(inst.data_size == 1) {
            uint8_t arg = load_operand_value8(cpu_state, inst, op);
            uint8_t res = arg + 1;
            store_operand_value8(cpu_state, inst, op, res);
            set_flags_szp_value8(cpu_state, res);
            set_flag_af_add(cpu_state, arg, 1);
        }
        else if(inst.data_size == 2) {
            uint16_t arg = load_operand_value16(cpu_state, inst, op);
            uint16_t res = arg + 1;
            store_operand_value16(cpu_state, inst, op, res);
            set_flags_szp_value16(cpu_state, res);
            set_flag_af_add(cpu_state, arg, 1);
        }
    }
    else if(inst.opcode == OPCODE_dec) {
        assert(inst.noperands == 1);
        assert(inst.operands[0].type != OPERAND_IMM);
        Inst_Operand op = inst.operands[0];
        if(inst.data_size == 1) {
            uint8_t arg = load_operand_value8(cpu_state, inst, op);
            uint8_t res = arg - 1;
            store_operand_value8(cpu_state, inst, op, res);
            set_flags_szp_value8(cpu_state, res);
            set_flag_af_sub(cpu_state, arg, 1);
        }
        else if(inst.data_size == 2) {
            uint16_t arg = load_operand_value16(cpu_state, inst, op);
            uint16_t res = arg - 1;
            store_operand_value16(cpu_state, inst, op, res);
            set_flags_szp_value16(cpu_state, res);
            set_flag_af_sub(cpu_state, arg, 1);
        }
    }
    else if(inst.opcode == OPCODE_neg) {
        assert(inst.noperands == 1);
        assert(inst.operands[0].type != OPERAND_IMM);
        Inst_Operand op = inst.operands[0];
        if(inst.data_size == 1) {
            uint8_t arg = load_operand_value8(cpu_state, inst, op);
            uint8_t res = ~arg+1;
            store_operand_value8(cpu_state, inst, op, res);
            set_flags_szp_value8(cpu_state, res);
            // TODO fix CO flags
            cpu_state->regs[REG_FL] &= ~FLAG_CF;
            if(res > 0) cpu_state->regs[REG_FL] |= FLAG_CF;
            set_flag_af_sub(cpu_state, 0, arg); // TODO check
        }
        else if(inst.data_size == 2) {
            uint16_t arg = load_operand_value16(cpu_state, inst, op);
            uint16_t res = ~arg+1;
            store_operand_value16(cpu_state, inst, op, res);
            // TODO fix CO flags
            cpu_state->regs[REG_FL] &= ~FLAG_CF;
            if(res > 0) cpu_state->regs[REG_FL] |= FLAG_CF;
            set_flags_szp_value16(cpu_state, res);
            set_flag_af_sub(cpu_state, 0, arg); // TODO check
        }
    }
    else if(inst.opcode == OPCODE_not) {
        assert(inst.noperands == 1);
        assert(inst.operands[0].type != OPERAND_IMM);
        Inst_Operand op = inst.operands[0];
        if(inst.data_size == 1) {
            uint8_t arg = load_operand_value8(cpu_state, inst, op);
            uint8_t res = ~arg;
            store_operand_value8(cpu_state, inst, op, res);
        }
        else if(inst.data_size == 2) {
            uint16_t arg = load_operand_value16(cpu_state, inst, op);
            uint16_t res = ~arg;
            store_operand_value16(cpu_state, inst, op, res);
        }
    }
    // INST_1ST(shl, B(110100), F_V, F_W, F_MOD, B(100), F_RM)
    // INST_1ST(shr, B(110100), F_V, F_W, F_MOD, B(101), F_RM)
    // INST_1ST(sar, B(110100), F_V, F_W, F_MOD, B(111), F_RM)
    // INST_1ST(rol, B(110100), F_V, F_W, F_MOD, B(000), F_RM)
    // INST_1ST(ror, B(110100), F_V, F_W, F_MOD, B(001), F_RM)
    // INST_1ST(rcl, B(110100), F_V, F_W, F_MOD, B(010), F_RM)
    // INST_1ST(rcr, B(110100), F_V, F_W, F_MOD, B(011), F_RM)
    else if(inst.opcode == OPCODE_shl) {
        assert(inst.noperands == 2);
        assert(inst.operands[0].type != OPERAND_IMM);
        Inst_Operand dst = inst.operands[0];
        Inst_Operand src = inst.operands[1];
        if(inst.data_size == 1) {
            uint8_t arg0 = load_operand_value8(cpu_state, inst, dst);
            uint8_t arg1 = load_operand_value8(cpu_state, inst, src);
            arg1 &= 0x1f;
            uint8_t res = arg0 << arg1;
            store_operand_value8(cpu_state, inst, dst, res);
            set_flags_szp_value8(cpu_state, res);
            if(arg1 != 0) {
                cpu_state->regs[REG_FL] &= ~FLAG_CF;
                if(arg0 & 0x80) {
                    cpu_state->regs[REG_FL] |= FLAG_CF;
                }
                if(arg1 == 1) {
                    cpu_state->regs[REG_FL] &= ~FLAG_OF;
                    if((arg0 & 0x80) == (res & 0x80)) {
                        cpu_state->regs[REG_FL] |= FLAG_OF;
                    }
                }
            }
        }
        else if(inst.data_size == 2) {
            uint16_t arg0 = load_operand_value16(cpu_state, inst, dst);
            uint8_t arg1 = load_operand_value8(cpu_state, inst, src);
            arg1 &= 0x1f;
            uint16_t res = arg0 << arg1;
            store_operand_value16(cpu_state, inst, dst, res);
            set_flags_szp_value16(cpu_state, res);
            if(arg1 != 0) {
                cpu_state->regs[REG_FL] &= ~FLAG_CF;
                if(arg0 & 0x8000) {
                    cpu_state->regs[REG_FL] |= FLAG_CF;
                }
                if(arg1 == 1) {
                    cpu_state->regs[REG_FL] &= ~FLAG_OF;
                    if((arg0 & 0x8000) == (res & 0x8000)) {
                        cpu_state->regs[REG_FL] |= FLAG_OF;
                    }
                }
            }
        }
    }
    else if(inst.opcode == OPCODE_shr) {
        assert(inst.noperands == 2);
        assert(inst.operands[0].type != OPERAND_IMM);
        Inst_Operand dst = inst.operands[0];
        Inst_Operand src = inst.operands[1];
        if(inst.data_size == 1) {
            int8_t arg0 = load_operand_value8(cpu_state, inst, dst);
            uint8_t arg1 = load_operand_value8(cpu_state, inst, src);
            arg1 &= 0x1f;
            int8_t res = arg0 >> arg1;
            store_operand_value8(cpu_state, inst, dst, res);
            set_flags_szp_value8(cpu_state, res);
            if(arg1 != 0) {
                cpu_state->regs[REG_FL] &= ~FLAG_CF;
                if(arg0 & 0x01) {
                    cpu_state->regs[REG_FL] |= FLAG_CF;
                }
                if(arg1 == 1) {
                    cpu_state->regs[REG_FL] &= ~FLAG_OF;
                    if(arg0 & 0x80) {
                        cpu_state->regs[REG_FL] |= FLAG_OF;
                    }
                }
            }
        }
        else if(inst.data_size == 2) {
            int16_t arg0 = load_operand_value16(cpu_state, inst, dst);
            uint8_t arg1 = load_operand_value8(cpu_state, inst, src);
            arg1 &= 0x1f;
            int16_t res = arg0 >> arg1;
            store_operand_value16(cpu_state, inst, dst, res);
            set_flags_szp_value16(cpu_state, res);
            if(arg1 != 0) {
                cpu_state->regs[REG_FL] &= ~FLAG_CF;
                if(arg0 & 0x0001) {
                    cpu_state->regs[REG_FL] |= FLAG_CF;
                }
                if(arg1 == 1) {
                    cpu_state->regs[REG_FL] &= ~FLAG_OF;
                    if(arg0 & 0x8000) {
                        cpu_state->regs[REG_FL] |= FLAG_OF;
                    }
                }
            }
        }
    }
    else if(inst.opcode == OPCODE_sar) {
        assert(inst.noperands == 2);
        assert(inst.operands[0].type != OPERAND_IMM);
        Inst_Operand dst = inst.operands[0];
        Inst_Operand src = inst.operands[1];
        if(inst.data_size == 1) {
            uint8_t arg0 = load_operand_value8(cpu_state, inst, dst);
            uint8_t arg1 = load_operand_value8(cpu_state, inst, src);
            arg1 &= 0x1f;
            uint8_t res = arg0 >> arg1;
            store_operand_value8(cpu_state, inst, dst, res);
            set_flags_szp_value8(cpu_state, res);
            if(arg1 != 0) {
                cpu_state->regs[REG_FL] &= ~FLAG_CF;
                if(arg0 & 0x01) {
                    cpu_state->regs[REG_FL] |= FLAG_CF;
                }
                if(arg1 == 1) {
                    cpu_state->regs[REG_FL] &= ~FLAG_OF;
                }
            }
        }
        else if(inst.data_size == 2) {
            uint16_t arg0 = load_operand_value16(cpu_state, inst, dst);
            uint8_t arg1 = load_operand_value8(cpu_state, inst, src);
            arg1 &= 0x1f;
            uint16_t res = arg0 >> arg1;
            store_operand_value16(cpu_state, inst, dst, res);
            set_flags_szp_value16(cpu_state, res);
            if(arg1 != 0) {
                cpu_state->regs[REG_FL] &= ~FLAG_CF;
                if(arg0 & 0x0001) {
                    cpu_state->regs[REG_FL] |= FLAG_CF;
                }
                if(arg1 == 1) {
                    cpu_state->regs[REG_FL] &= ~FLAG_OF;
                }
            }
        }
    }
    else if(inst.opcode == OPCODE_rol) {
        assert(inst.noperands == 2);
        assert(inst.operands[0].type != OPERAND_IMM);
        Inst_Operand dst = inst.operands[0];
        Inst_Operand src = inst.operands[1];
        if(inst.data_size == 1) {
            uint8_t arg0 = load_operand_value8(cpu_state, inst, dst);
            uint8_t arg1 = load_operand_value8(cpu_state, inst, src);
            uint8_t count = (arg1&0x1f) % 8;
            uint8_t res = (arg0 << count | (arg0 >> (8 - count)));
            store_operand_value8(cpu_state, inst, dst, res);
            set_flags_szp_value8(cpu_state, res);
            if(count != 0) {
                cpu_state->regs[REG_FL] &= ~FLAG_CF;
                if(res & 0x80) {
                    cpu_state->regs[REG_FL] |= FLAG_CF;
                }
                if(count == 1) {
                    cpu_state->regs[REG_FL] &= ~FLAG_OF;
                    if((arg0 & 0x80) ^ (res & 0x80)) {
                        cpu_state->regs[REG_FL] |= FLAG_OF;
                    }
                }
            }
        }
        else if(inst.data_size == 2) {
            uint16_t arg0 = load_operand_value16(cpu_state, inst, dst);
            uint8_t arg1 = load_operand_value8(cpu_state, inst, src);
            arg1 &= 0x1f;
            uint16_t res = arg0 << arg1;
            store_operand_value16(cpu_state, inst, dst, res);
            set_flags_szp_value16(cpu_state, res);
            if(arg1 != 0) {
                cpu_state->regs[REG_FL] &= ~FLAG_CF;
                if(arg0 & 0x8000) {
                    cpu_state->regs[REG_FL] |= FLAG_CF;
                }
                if(arg1 == 1) {
                    cpu_state->regs[REG_FL] &= ~FLAG_OF;
                    if((arg0 & 0x8000) ^ (res & 0x8000)) {
                        cpu_state->regs[REG_FL] |= FLAG_OF;
                    }
                }
            }
        }        
    }
    else if(inst.opcode == OPCODE_ror) {
        // TODO
    }
    else if(inst.opcode == OPCODE_rcl) {
        // TODO
    }
    else if(inst.opcode == OPCODE_rcr) {
        // TODO
    }
    else if(inst.opcode == OPCODE_jmp) {
        assert(inst.noperands == 1);
        cpu_jmp_to_op(cpu_state, inst, inst.operands[0], false);
    }
    else if(inst.opcode == OPCODE_jmpf) {
        assert(inst.noperands == 1);
        cpu_jmp_to_op(cpu_state, inst, inst.operands[0], true);
    }
    else if(inst.opcode == OPCODE_jz) {
        assert(inst.noperands == 1);
        if(cpu_state->regs[REG_FL] & FLAG_ZF) {
            cpu_jmp_to_op(cpu_state, inst, inst.operands[0], false);
        }
    }
    else if(inst.opcode == OPCODE_jl) {
        assert(inst.noperands == 1);
        if(!(cpu_state->regs[REG_FL] & FLAG_SF) != !(cpu_state->regs[REG_FL] & FLAG_OF)) {
            cpu_jmp_to_op(cpu_state, inst, inst.operands[0], false);
        }
    }
    else if(inst.opcode == OPCODE_jle) {
        assert(inst.noperands == 1);
        if((cpu_state->regs[REG_FL] & FLAG_ZF) || (!(cpu_state->regs[REG_FL] & FLAG_SF) != !(cpu_state->regs[REG_FL] & FLAG_OF))) {
            cpu_jmp_to_op(cpu_state, inst, inst.operands[0], false);
        }
    }
    else if(inst.opcode == OPCODE_jb) {
        assert(inst.noperands == 1);
        if(cpu_state->regs[REG_FL] & FLAG_CF) {
            cpu_jmp_to_op(cpu_state, inst, inst.operands[0], false);
        }
    }
    else if(inst.opcode == OPCODE_jbe) {
        assert(inst.noperands == 1);
        if(cpu_state->regs[REG_FL] & (FLAG_CF | FLAG_ZF)) {
            cpu_jmp_to_op(cpu_state, inst, inst.operands[0], false);
        }
    }
    else if(inst.opcode == OPCODE_jp) {
        assert(inst.noperands == 1);
        if(cpu_state->regs[REG_FL] & FLAG_PF) {
            cpu_jmp_to_op(cpu_state, inst, inst.operands[0], false);
        }
    }
    else if(inst.opcode == OPCODE_jo) {
        assert(inst.noperands == 1);
        if(cpu_state->regs[REG_FL] & FLAG_OF) {
            cpu_jmp_to_op(cpu_state, inst, inst.operands[0], false);
        }
    }
    else if(inst.opcode == OPCODE_js) {
        assert(inst.noperands == 1);
        if(cpu_state->regs[REG_FL] & FLAG_SF) {
            cpu_jmp_to_op(cpu_state, inst, inst.operands[0], false);
        }
    }
    else if(inst.opcode == OPCODE_jnz) {
        assert(inst.noperands == 1);
        if((cpu_state->regs[REG_FL] & FLAG_ZF) == 0) {
            cpu_jmp_to_op(cpu_state, inst, inst.operands[0], false);
        }
    }
    else if(inst.opcode == OPCODE_jge) {
        assert(inst.noperands == 1);
        if(!(cpu_state->regs[REG_FL] & FLAG_SF) == !(cpu_state->regs[REG_FL] & FLAG_OF)) {
            cpu_jmp_to_op(cpu_state, inst, inst.operands[0], false);
        }
    }
    else if(inst.opcode == OPCODE_jg) {
        assert(inst.noperands == 1);
        if(((cpu_state->regs[REG_FL] & FLAG_ZF) == 0) || (!(cpu_state->regs[REG_FL] & FLAG_SF) == !(cpu_state->regs[REG_FL] & FLAG_OF))) {
            cpu_jmp_to_op(cpu_state, inst, inst.operands[0], false);
        }
    }
    else if(inst.opcode == OPCODE_jae) {
        assert(inst.noperands == 1);
        if((cpu_state->regs[REG_FL] & FLAG_CF) == 0) {
            cpu_jmp_to_op(cpu_state, inst, inst.operands[0], false);
        }
    }
    else if(inst.opcode == OPCODE_ja) {
        assert(inst.noperands == 1);
        if((cpu_state->regs[REG_FL] & (FLAG_CF | FLAG_ZF)) == 0) {
            cpu_jmp_to_op(cpu_state, inst, inst.operands[0], false);
        }
    }
    else if(inst.opcode == OPCODE_jnp) {
        assert(inst.noperands == 1);
        if((cpu_state->regs[REG_FL] & FLAG_PF) == 0) {
            cpu_jmp_to_op(cpu_state, inst, inst.operands[0], false);
        }
    }
    else if(inst.opcode == OPCODE_jno) {
        assert(inst.noperands == 1);
        if((cpu_state->regs[REG_FL] & FLAG_OF) == 0) {
            cpu_jmp_to_op(cpu_state, inst, inst.operands[0], false);
        }
    }
    else if(inst.opcode == OPCODE_jns) {
        assert(inst.noperands == 1);
        if((cpu_state->regs[REG_FL] & FLAG_SF) == 0) {
            cpu_jmp_to_op(cpu_state, inst, inst.operands[0], false);
        }
    }
    else if(inst.opcode == OPCODE_jcxz) {
        assert(inst.noperands == 1);
        if(cpu_state->regs[REG_C] == 0) {
            cpu_jmp_to_op(cpu_state, inst, inst.operands[0], false);
        }
    }
    else if(inst.opcode == OPCODE_loop) {
        assert(inst.noperands == 1);
        cpu_state->regs[REG_C] -= 1;
        if(cpu_state->regs[REG_C] != 0) {
            cpu_jmp_to_op(cpu_state, inst, inst.operands[0], false);
        }
    }
    else if(inst.opcode == OPCODE_loopz) {
        assert(inst.noperands == 1);
        cpu_state->regs[REG_C] -= 1;
        if(cpu_state->regs[REG_C] != 0 && (cpu_state->regs[REG_FL] & FLAG_ZF)) {
            cpu_jmp_to_op(cpu_state, inst, inst.operands[0], false);
        }
    }
    else if(inst.opcode == OPCODE_loopnz) {
        assert(inst.noperands == 1);
        cpu_state->regs[REG_C] -= 1;
        if(cpu_state->regs[REG_C] != 0 && !(cpu_state->regs[REG_FL] & FLAG_ZF)) {
            cpu_jmp_to_op(cpu_state, inst, inst.operands[0], false);
        }
    }
    else if(inst.opcode == OPCODE_push) {
        assert(inst.noperands == 1);
        Inst_Operand op = inst.operands[0];
        uint16_t word;
        if(inst.data_size == 8) {
            word = load_operand_value8(cpu_state, inst, op);
        }
        else if(inst.data_size == 16) {
            word = load_operand_value16(cpu_state, inst, op);
        }
        uint16_t seg = cpu_state->regs[REG_CS];
        uint16_t offs = cpu_state->regs[REG_IP];
        offs -= 2;
        mem_store8_at_far(cpu_state->memory, (Far_Ptr){.seg = seg, .offs = offs}, 0, word);
    }
    else if(inst.opcode == OPCODE_pop) {
        assert(inst.noperands == 1);
        Inst_Operand op = inst.operands[0];
        uint16_t seg = cpu_state->regs[REG_CS];
        uint16_t offs = cpu_state->regs[REG_IP];
        uint16_t word = mem_load8_at_far(cpu_state->memory, (Far_Ptr){.seg = seg, .offs = offs}, 0);
        offs += 2;
        if(inst.data_size == 8) {
            store_operand_value8(cpu_state, inst, op, (uint8_t)word);
        }
        else if(inst.data_size == 16) {
            store_operand_value16(cpu_state, inst, op, word);
        }
    }
}
