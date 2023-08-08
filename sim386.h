#pragma once

#include <stdint.h>

#define STACK_BASE 0x4000000

#define GET_PDE(addr) (addr >> 22)
#define GET_PTE(addr) ((addr & 0x3ff000) >> 12)

#define MOD(modrm) ((modrm & 0xc0) >> 6)
#define REG(modrm) ((modrm & 0x38) >> 3)
#define RM(modrm) (modrm & 0x7)

#define print_modrm()				printf("Mod=%d Reg=%d Rm=%d\n", MOD(modrm), REG(modrm), RM(modrm));

#define get_modrm_src_reg_1632()	uint32_t* src_ptr = get_reg_1632(cpu, REG(modrm));

#define finish_op(op16, op32)		switch(cpu->operand_size){ \
										case 0: \
											op16(cpu, (uint16_t*)dst_ptr, (uint16_t*)src_ptr); \
											break; \
										case 1: \
											op32(cpu, dst_ptr, src_ptr); \
											break; \
									}

#define get_modrm_dst_ptr(c)		uint32_t* dst_ptr; \
									switch(MOD(modrm)){ \
										case 3: \
											dst_ptr = get_reg_1632(cpu, RM(modrm)); \
											printf("%s", tables[cpu->operand_size][RM(modrm)]); \
											break; \
										default: \
											dst_ptr = (uint32_t*)virtual_to_physical_addr(cpu, calc_modrm_addr(cpu, modrm)); \
											break; \
									} \
									if (c){ printf(", "); }

#define get_modrm()					uint8_t modrm = *(virtual_to_physical_addr(cpu, cpu->eip + 1));

typedef struct{
	uint8_t* entries[1024]; //points to an address in physical memory
} i386_PT;

typedef struct{
	i386_PT* entries[1024];
} i386_PD;

typedef struct{
	struct{
		union{
			struct{
				uint8_t al;
				uint8_t ah;
			};
			uint16_t ax;
			uint32_t eax;
		};
	};

	struct{
		union{
			struct{
				uint8_t bl;
				uint8_t bh;
			};
			uint16_t bx;
			uint32_t ebx;
		};
	};

	struct{
		union{
			struct{
				uint8_t cl;
				uint8_t ch;
			};
			uint16_t cx;
			uint32_t ecx;
		};
	};

	struct{
		union{
			struct{
				uint8_t dl;
				uint8_t dh;
			};
			uint16_t dx;
			uint32_t edx;
		};
	};

	//segment registers are basically ignored here
	uint16_t ss;
	uint16_t ds;
	uint16_t cs;
	uint16_t es;

	union{
		uint16_t si;
		uint32_t esi;
	};

	union{
		uint16_t di;
		uint32_t edi;
	};

	union{
		uint16_t ip;
		uint32_t eip;
	};

	union{
		uint16_t sp;
		uint32_t esp;
	};

	union{
		uint16_t bp;
		uint32_t ebp;
	};

	union{
		uint16_t flags;
		uint32_t eflags;
	};

	int running;
	int operand_size; //0 for 16-bit, 1 for 32-bit

	i386_PD page_dir;
} i386; //address size?

void map_section(i386* cpu, uint32_t vaddr, uint8_t* paddr, uint32_t size);
uint8_t* virtual_to_physical_addr(i386* cpu, uint32_t vaddr);
void virtual_mmap(i386* cpu, uint32_t vaddr, uint8_t* paddr);
void cpu_init(i386* cpu);
void cpu_step(i386* cpu);
void cpu_dump(i386* cpu);
void handle_syscall(i386* cpu);