
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <malloc.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "types.c"
#include "memory.c"

#include "decoder_types.c"
#include "decoder.c"
#include "8086.c"
#include "printing.c"

int main(int argc, char **args) {
    char *file_path = NULL;
    if(argc > 1) {
        file_path = args[0];
    }
    if(file_path == NULL) {
        fprintf(stderr, "Error: No filename provided\n");
        exit(1);
    }
    FILE *code = fopen("code", "rb");
    Memory mem = mem_init();
    Far_Ptr load_addr = {0x0000, 0x7c00};
    uint32_t file_size = mem_load_file_at(&mem, code, load_addr);
    // print_instructions(&mem, load_addr, file_size);
    CPU_State cpu_state = cpu_init(&mem, load_addr);
    for(;;) {
        Far_Ptr cpu_addr = {.seg = cpu_state.regs[REG_CS], .offs = cpu_state.regs[REG_IP]};
        if(mem_lin_addr(cpu_addr) >= mem_lin_addr(load_addr) + file_size) {
            break;
        }
        Instruction inst;
        CPU_State before = cpu_state;
        cpu_exec(&cpu_state, &inst);
        CPU_State after = cpu_state;
        inst_print(&mem, inst, inst.addr);
        cpu_state_diff_print(before, after);
        putchar('\n');
    }
    cpu_regs_print(&cpu_state);
    return 0;
}

