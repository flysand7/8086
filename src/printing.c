
static char *get_reg_name16(Reg_Index idx) {
    char *names16[] = {
        "ax", "cx", "dx", "bx",
        "sp", "bp", "si", "di",
        "es", "cs", "ss", "ds",
        "ip", "fl",
    };
    return names16[idx];
}

static char *get_reg_name(Reg_Operand reg) {
    if(reg.size == 2) {
        return get_reg_name16(reg.idx);
    }
    else if(reg.size == 1) {
        if(reg.offset == 0) {
            char *names_lo[] = {
                "al", "cl", "dl", "bl",
            };
            return names_lo[reg.idx];
        }
        else if(reg.offset == 1) {
            char *names_hi[] = {
                "ah", "ch", "dh", "bh",
            };
            return names_hi[reg.idx];
        }
    }
    return "<?reg>";
}

static void print_operand(Instruction inst, Inst_Operand operand) {
    switch(operand.type) {
        case OPERAND_REG: {
            char *reg_name = get_reg_name(operand.reg);
            printf("%s", reg_name);
        } break;
        case OPERAND_MEM: {
            char *reg1_name = NULL;
            char *reg2_name = NULL;
            if(inst.seg_override != REG_ZERO) {
                printf("%s:", get_reg_name16(inst.seg_override));
            }
            putchar('[');
            if(operand.mem.reg1 != REG_ZERO) {
                reg1_name = get_reg_name16(operand.mem.reg1);
            }
            if(operand.mem.reg2 != REG_ZERO) {
                reg2_name = get_reg_name16(operand.mem.reg2);
            }
            bool something_before = false;
            if(reg1_name != NULL) {
                printf("%s", reg1_name);
                something_before = true;
            }
            if(reg2_name != NULL) {
                if(something_before) {
                    printf(" + ");
                }
                printf("%s", reg2_name);
                something_before = true;
            }
            if(operand.mem.disp != 0) {
                int16_t disp = operand.mem.disp;
                // bool negative = false;
                // if(disp < 0) {
                //     disp = -disp;
                //     negative = true;
                // }
                // if(something_before) {
                //     printf(negative? " - " : " + ");
                // }
                // disp = -disp;
                if(something_before) {
                    printf(" + ");
                }
                printf("0x%04x", disp);
            }
            putchar(']');
        } break;
        case OPERAND_IMM: {
            if(operand.imm.size == 1) {
                printf("0x%02x", operand.imm.value);
            }
            else if(operand.imm.size == 2) {
                printf("0x%04x", operand.imm.value);
            }
        } break;
        case OPERAND_ADDR: {
            Addr_Type type = operand.addr.type;
            if(type == ADDR_SHORT) {
                // Far_Ptr inst_addr = inst.addr;
                // inst_addr.offs += operand.addr.offs8;
                // printf("[0x%04x:0x%04x]", inst_addr.seg, inst_addr.offs);
                printf("%+d", operand.addr.offs8);
            }
            else if(type == ADDR_NEAR) {
                bool negative = false;
                int16_t offs = operand.addr.offs;
                if(offs < 0) {
                    offs = -offs;
                    negative = true;
                }
                offs = -offs;
                printf(negative? "-" : "+");
                printf("0x%04x", operand.addr.offs16);
            }
            else if(type == ADDR_FAR) {
                printf("0x%04x:0x%04x", operand.addr.seg, operand.addr.offs);
            }
        } break;
        default: {
            assert(false);
        }
    }
}

static void inst_print(Memory *mem, Instruction inst, Far_Ptr at) {
    // Print instruction address
    printf("  %04x:%04x  ", at.seg, at.offs);
    // Print instruction bytes
    uint32_t inst_bytes = inst.bytes;
    uint32_t inst_max_bytes = 6;
    uint32_t inst_lin_addr = mem_lin_addr(inst.addr);
    for(uint32_t i = 0; i != inst_bytes; ++i) {
        uint8_t byte = mem_load8_at_lin(mem, inst_lin_addr + i);
        printf("%02x", byte);
    }
    if(inst_bytes < inst_max_bytes) {
        for(uint32_t i = 0; i != inst_max_bytes - inst_bytes; ++i) {
            printf("  ");
        }
    }
    printf("  ");
    // Print opcode with prefixes
    if(inst.lock_prefix) {
        printf("lock ");
    }
    if(inst.rep_prefix) {
        printf("rep ");
    }
    printf("%s", g_inst_opcode_names[inst.opcode]);
    if(inst.z_flag) {
        printf("z");
    }
    for(int i = 0; i < inst.noperands; ++i) {
        if(i != 0) {
            putchar(',');
        }
        putchar(' ');
        print_operand(inst, inst.operands[i]);
    }
}

static Far_Ptr advance_address(Far_Ptr ptr, uint32_t bytes) {
    uint32_t linear_addr = mem_lin_addr(ptr);
    linear_addr += bytes;
    return mem_far_addr(linear_addr);
}

static void print_instructions(Memory *mem, Far_Ptr at, uint32_t total_bytes) {
    DecodeCtx ctx = {0};
    ctx.mem = mem;
    ctx.start_addr = at;
    uint32_t bytes_printed = 0;
    while(bytes_printed < total_bytes) {
        Instruction inst = inst_decode(&ctx);
        if(inst.bytes > 0) {
            if(inst.printable) {
                inst_print(mem, inst, ctx.inst_addr);
                putchar('\n');
            }
            bytes_printed += inst.bytes;
            ctx.start_addr = advance_address(ctx.start_addr, inst.bytes);
        }
        else {
            fprintf(stderr, "Failed to decode instruction at %04x:%04x\n", ctx.inst_addr.seg, ctx.inst_addr.offs);
            exit(1);
        }
    }
}

static void cpu_regs_print(CPU_State *cpu_state) {
    printf("General-Purpose registers:\n");
    printf("  AX: %04x    SP: %04x\n", cpu_state->regs[REG_A], cpu_state->regs[REG_SP]);
    printf("  CX: %04x    BP: %04x\n", cpu_state->regs[REG_C], cpu_state->regs[REG_BP]);
    printf("  DX: %04x    SI: %04x\n", cpu_state->regs[REG_D], cpu_state->regs[REG_SI]);
    printf("  BX: %04x    DI: %04x\n", cpu_state->regs[REG_B], cpu_state->regs[REG_DI]);
    printf("Segment registers:\n");
    printf("  ES: %04x  SS: %04x\n", cpu_state->regs[REG_ES], cpu_state->regs[REG_SS]);
    printf("  CS: %04x  DS: %04x\n", cpu_state->regs[REG_CS], cpu_state->regs[REG_DS]);
    printf("CPU state:\n");
    printf("  IP: %04x:%04x\n", cpu_state->regs[REG_CS], cpu_state->regs[REG_IP]);
    printf("  FL: %s%s%s%s%s%s\n",
        (((cpu_state->regs[REG_FL] & FLAG_CF) >>  0)? "C" : ""),
        (((cpu_state->regs[REG_FL] & FLAG_PF) >>  2)? "P" : ""),
        (((cpu_state->regs[REG_FL] & FLAG_AF) >>  4)? "A" : ""),
        (((cpu_state->regs[REG_FL] & FLAG_ZF) >>  6)? "Z" : ""),
        (((cpu_state->regs[REG_FL] & FLAG_SF) >>  7)? "S" : ""),
        (((cpu_state->regs[REG_FL] & FLAG_OF) >> 11)? "O" : ""));
}

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>

    static void fill_until_column(int column) {
        CONSOLE_SCREEN_BUFFER_INFO ci;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ci);
        uint32_t cur_column = ci.dwCursorPosition.X;
        if(cur_column < column) {
            for(int i = 0; i < column - cur_column; ++i) {
                putchar(' ');
            }
        }
    }
#else
    static void fill_until_column(int column) {
        // dunno haven't checked, doesn't work on
        // windows and I don't want to figure it out
        // on other platforms
        printf("\x1b[;%df", column);
    }
#endif

static void cpu_state_diff_print(CPU_State before, CPU_State after) {
    fill_until_column(60);
    printf(";; ");
    if(before.regs[REG_A] != after.regs[REG_A]) {
        printf("AX: %04x->%04x, ", before.regs[REG_A], after.regs[REG_A]);
    }
    if(before.regs[REG_C] != after.regs[REG_C]) {
        printf("CX: %04x->%04x, ", before.regs[REG_C], after.regs[REG_C]);
    }
    if(before.regs[REG_D] != after.regs[REG_D]) {
        printf("DX: %04x->%04x, ", before.regs[REG_D], after.regs[REG_D]);
    }
    if(before.regs[REG_B] != after.regs[REG_B]) {
        printf("BX: %04x->%04x, ", before.regs[REG_B], after.regs[REG_B]);
    }
    if(before.regs[REG_SP] != after.regs[REG_SP]) {
        printf("SP: %04x->%04x, ", before.regs[REG_SP], after.regs[REG_SP]);
    }
    if(before.regs[REG_BP] != after.regs[REG_BP]) {
        printf("BP: %04x->%04x, ", before.regs[REG_BP], after.regs[REG_BP]);
    }
    if(before.regs[REG_SI] != after.regs[REG_SI]) {
        printf("SI: %04x->%04x, ", before.regs[REG_SI], after.regs[REG_SI]);
    }
    if(before.regs[REG_DI] != after.regs[REG_DI]) {
        printf("DI: %04x->%04x, ", before.regs[REG_DI], after.regs[REG_DI]);
    }
    if(before.regs[REG_ES] != after.regs[REG_ES]) {
        printf("ES: %04x->%04x, ", before.regs[REG_ES], after.regs[REG_ES]);
    }
    if(before.regs[REG_CS] != after.regs[REG_CS]) {
        printf("CS: %04x->%04x, ", before.regs[REG_CS], after.regs[REG_CS]);
    }
    if(before.regs[REG_SS] != after.regs[REG_SS]) {
        printf("SS: %04x->%04x, ", before.regs[REG_SS], after.regs[REG_SS]);
    }
    if(before.regs[REG_DS] != after.regs[REG_DS]) {
        printf("DS: %04x->%04x, ", before.regs[REG_DS], after.regs[REG_DS]);
    }
    uint16_t add_flags = (~before.regs[REG_FL]) & after.regs[REG_FL];
    uint16_t rem_flags = before.regs[REG_FL] & (~after.regs[REG_FL]);
    if(add_flags != 0 || rem_flags != 0) {
        printf("FL: ");
    }
    if(add_flags != 0) {
        printf("+%s%s%s%s%s%s",
            (((add_flags & FLAG_CF) >>  0)? "C" : ""),
            (((add_flags & FLAG_PF) >>  2)? "P" : ""),
            (((add_flags & FLAG_AF) >>  4)? "A" : ""),
            (((add_flags & FLAG_ZF) >>  6)? "Z" : ""),
            (((add_flags & FLAG_SF) >>  7)? "S" : ""),
            (((add_flags & FLAG_OF) >> 11)? "O" : ""));
    }
    if(rem_flags != 0) {
        printf("-%s%s%s%s%s%s",
            (((rem_flags & FLAG_CF) >>  0)? "C" : ""),
            (((rem_flags & FLAG_PF) >>  2)? "P" : ""),
            (((rem_flags & FLAG_AF) >>  4)? "A" : ""),
            (((rem_flags & FLAG_ZF) >>  6)? "Z" : ""),
            (((rem_flags & FLAG_SF) >>  7)? "S" : ""),
            (((rem_flags & FLAG_OF) >> 11)? "O" : ""));
    }
}
