
#define MEMORY_SIZE 1*mb
#define MEMORY_MASK 0xfffff

static Memory mem_init() {
    Memory mem;
    mem.buffer = malloc(MEMORY_SIZE);
    if(mem.buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for 8086\n");
        exit(1);
    }
    return mem;
}

static uint32_t mem_lin_addr(Far_Ptr ptr) {
    return (ptr.seg << 4) + ptr.offs;
}

static Far_Ptr mem_far_addr(uint32_t lin_addr) {
    uint16_t seg = (lin_addr & 0xf0000) >> 4;
    uint16_t offs = lin_addr & 0x0ffff;
    Far_Ptr ptr;
    ptr.seg = seg;
    ptr.offs = offs;
    return ptr;
}

static void mem_store8_at_lin(Memory *mem, uint32_t lin_addr, uint8_t value) {
    uint32_t norm_addr = lin_addr & MEMORY_MASK;
    if(norm_addr >= MEMORY_SIZE) {
        fprintf(stderr, "Fatal: bad memory mask");
        exit(1);
    }
    mem->buffer[norm_addr] = value;
}

static uint8_t mem_load8_at_lin(Memory *mem, uint32_t lin_addr) {
    uint32_t norm_addr = lin_addr & MEMORY_MASK;
    if(norm_addr >= MEMORY_SIZE) {
        fprintf(stderr, "Fatal: bad memory mask");
        exit(1);
    }
    return mem->buffer[norm_addr];
}

static void mem_store16_at_lin(Memory *mem, uint32_t lin_addr, uint16_t value) {
    uint8_t lo = value & 0xff;
    uint8_t hi = value >> 8;
    mem_store8_at_lin(mem, lin_addr, lo);
    mem_store8_at_lin(mem, lin_addr+1, hi);
}

static uint16_t mem_load16_at_lin(Memory *mem, uint32_t lin_addr) {
    uint16_t lo = mem_load8_at_lin(mem, lin_addr);
    uint16_t hi = mem_load8_at_lin(mem, lin_addr+1);
    return lo | (hi << 8);
}


static void mem_store8_at_far(Memory *mem, Far_Ptr ptr, int32_t offset, uint8_t value) {
    uint32_t lin_addr = mem_lin_addr(ptr) + offset;
    mem_store8_at_lin(mem, lin_addr, value);
}

static uint8_t mem_load8_at_far(Memory *mem, Far_Ptr ptr, int32_t offset) {
    uint32_t lin_addr = mem_lin_addr(ptr) + offset;
    return mem_load8_at_lin(mem, lin_addr);
}

static void mem_store16_at_far(Memory *mem, Far_Ptr ptr, int32_t offset, uint16_t value) {
    uint32_t lin_addr = mem_lin_addr(ptr) + offset;
    uint8_t lo = value & 0xff;
    uint8_t hi = value >> 8;
    mem_store8_at_lin(mem, lin_addr, lo);
    mem_store8_at_lin(mem, lin_addr+1, hi);
}

static uint16_t mem_load16_at_far(Memory *mem, Far_Ptr ptr, int32_t offset) {
    uint32_t lin_addr = mem_lin_addr(ptr) + offset;
    uint16_t lo = mem_load8_at_lin(mem, lin_addr);
    uint16_t hi = mem_load8_at_lin(mem, lin_addr+1);
    return lo | (hi << 8);
}

static uint32_t mem_load_file_at(Memory *mem, FILE *file, Far_Ptr at) {
    // Get the file size
    fseek(file, 0, SEEK_END);
    uint32_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    // Load the buffer in memory
    uint32_t linear_addr = mem_lin_addr(at);
    if(linear_addr + file_size >= MEMORY_SIZE) {
        fprintf(stderr, "Bad file load address: %04x:%04x\n", at.seg, at.offs);
        exit(1);
    }
    void *load_addr = mem->buffer + linear_addr;
    fread(load_addr, file_size, 1, file);
    return file_size;
}
