#include "sim386.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const char* reg_names_8[8] = {"AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH"};
const char* reg_names_16[8] = {"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"};
const char* reg_names_32[8] = {"EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI"};
const char** tables[2] = { reg_names_16, reg_names_32 };

uint32_t alu_sub32(i386* cpu, uint32_t a, uint32_t b){ //also has to set flags
	uint32_t result = a - b;
	return result;
}

void cpu_mov16(i386* cpu, uint16_t* dst_ptr, uint16_t* src_ptr){
	*dst_ptr = *src_ptr;
}

void cpu_mov32(i386* cpu, uint32_t* dst_ptr, uint32_t* src_ptr){
	*dst_ptr = *src_ptr;
}

void cpu_add16(i386* cpu, uint16_t* dst_ptr, uint16_t* src_ptr){
	printf("16-bit addition is unimplemented!");
	cpu->running = 0;
}

void cpu_or16(i386* cpu, uint16_t* dst_ptr, uint16_t* src_ptr){
	printf("16-bit OR is unimplemented!");
	cpu->running = 0;
}

void cpu_adc16(i386* cpu, uint16_t* dst_ptr, uint16_t* src_ptr){
	printf("16-bit add with carry is unimplemented!");
	cpu->running = 0;
}

void cpu_sbb16(i386* cpu, uint16_t* dst_ptr, uint16_t* src_ptr){
	printf("16-bit subtract with borrow is unimplemented!");
	cpu->running = 0;
}

void cpu_and16(i386* cpu, uint16_t* dst_ptr, uint16_t* src_ptr){
	printf("16-bit AND is unimplemented!");
	cpu->running = 0;
}

void cpu_sub16(i386* cpu, uint16_t* dst_ptr, uint16_t* src_ptr){
	printf("16-bit subtraction is unimplemented!");
	cpu->running = 0;
}

void cpu_xor16(i386* cpu, uint16_t* dst_ptr, uint16_t* src_ptr){
	printf("16-bit XOR is unimplemented!");
	cpu->running = 0;
}

void cpu_cmp16(i386* cpu, uint16_t* dst_ptr, uint16_t* src_ptr){
	printf("16-bit comparison is unimplemented!");
	cpu->running = 0;
}

void cpu_add32(i386* cpu, uint32_t* dst_ptr, uint32_t* src_ptr){
	printf("32-bit addition is unimplemented!");
	cpu->running = 0;
}

void cpu_or32(i386* cpu, uint32_t* dst_ptr, uint32_t* src_ptr){
	printf("32-bit OR is unimplemented!");
	cpu->running = 0;
}

void cpu_adc32(i386* cpu, uint32_t* dst_ptr, uint32_t* src_ptr){
	printf("32-bit add with carry is unimplemented!");
	cpu->running = 0;
}

void cpu_sbb32(i386* cpu, uint32_t* dst_ptr, uint32_t* src_ptr){
	printf("32-bit subtract with borrow is unimplemented!");
	cpu->running = 0;
}

void cpu_and32(i386* cpu, uint32_t* dst_ptr, uint32_t* src_ptr){
	printf("32-bit AND is unimplemented!");
	cpu->running = 0;
}

void cpu_sub32(i386* cpu, uint32_t* dst_ptr, uint32_t* src_ptr){
	*dst_ptr = alu_sub32(cpu, *dst_ptr, *src_ptr);
}

void cpu_xor32(i386* cpu, uint32_t* dst_ptr, uint32_t* src_ptr){
	printf("32-bit XOR is unimplemented!");
	cpu->running = 0;
}

void cpu_cmp32(i386* cpu, uint32_t* dst_ptr, uint32_t* src_ptr){
	printf("32-bit comparison is unimplemented!");
	cpu->running = 0;
}

void cpu_inc32(i386* cpu, uint32_t* dst_ptr){
	printf("32-bit increment is unimplemented!");
	cpu->running = 0;
}

void cpu_dec32(i386* cpu, uint32_t* dst_ptr){
	printf("32-bit decrement is unimplemented!");
	cpu->running = 0;
}

void cpu_inc16(i386* cpu, uint16_t* dst_ptr){
	printf("16-bit increment is unimplemented!");
	cpu->running = 0;
}

void cpu_dec16(i386* cpu, uint16_t* dst_ptr){
	printf("16-bit decrement is unimplemented!");
	cpu->running = 0;
}

const char* arith_family_names[8] = {"ADD", "OR", "ADC", "SBB", "AND", "SUB", "XOR", "CMP"};
void(*arith_family_fns_16[8])(i386*, uint16_t*, uint16_t*) = {cpu_add16, cpu_or16, cpu_adc16, cpu_sbb16, cpu_and16, cpu_sub16, cpu_xor16, cpu_cmp16};
void(*arith_family_fns_32[8])(i386*, uint32_t*, uint32_t*) = {cpu_add32, cpu_or32, cpu_adc32, cpu_sbb32, cpu_and32, cpu_sub32, cpu_xor32, cpu_cmp32};

void cpu_push16(i386* cpu, uint16_t *val){
	cpu->esp -= 2;
	*(uint16_t*)virtual_to_physical_addr(cpu, cpu->esp) = *val;
}

void cpu_push32(i386* cpu, uint32_t *val){
	cpu->esp -= 4;
	*(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp) = *val;
}

uint32_t cpu_pop32(i386* cpu){
	uint32_t val = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp);
	cpu->esp += 4;
	return val;
}

void cpu_call(i386* cpu, uint32_t* new_eip){
	cpu_push32(cpu, &(cpu->eip));
	cpu->eip = *new_eip;
}

void cpu_jump(i386* cpu, uint32_t* new_eip){
	cpu->eip = *new_eip;
}

uint8_t* get_reg_8(i386* cpu, uint8_t reg){
	switch (reg){
		case 0:
			return &(cpu->al);
			break;
		case 1:
			return &(cpu->cl);
			break;
		case 2:
			return &(cpu->dl);
			break;
		case 3:
			return &(cpu->bl);
			break;
		case 4:
			return &(cpu->ah);
			break;
		case 5:
			return &(cpu->ch);
			break;
		case 6:
			return &(cpu->dh);
			break;
		case 7:
			return &(cpu->bh);
			break;
	}
}

uint32_t* get_reg_1632(i386* cpu, uint8_t reg){
	switch (reg){
		case 0:
			return &(cpu->eax);
			break;
		case 1:
			return &(cpu->ecx);
			break;
		case 2:
			return &(cpu->edx);
			break;
		case 3:
			return &(cpu->ebx);
			break;
		case 4:
			return &(cpu->esp);
			break;
		case 5:
			return &(cpu->ebp);
			break;
		case 6:
			return &(cpu->esi);
			break;
		case 7:
			return &(cpu->edi);
			break;
	}
}

const char* ff_family_names[8] = { "INC", "DEC", "CALL", 0, "JMP", 0, "PUSH", 0 };
void(*ff_family_fns_32[8])(i386*, uint32_t*) = { cpu_inc32, cpu_dec32, cpu_call, 0, cpu_jump, 0, cpu_push32, 0 };
void(*ff_family_fns_16[8])(i386*, uint16_t*) = { cpu_inc16, cpu_dec16, 0, 0, 0, 0, 0, 0 };

uint32_t calc_modrm_addr(i386* cpu, uint8_t modrm){
	uint32_t addr = *(get_reg_1632(cpu, RM(modrm)));
	uint32_t* disp = (uint32_t*)virtual_to_physical_addr(cpu, cpu->eip);

	printf("[");

	switch (MOD(modrm)){
		case 0:
			if (RM(modrm) == 4){
				printf("SIB unimplemented!");
				cpu->running = 0;
				return 0xFFFFFFFF;
			}
			else if (RM(modrm) == 5){ //disp32 only
				addr = *disp;
				cpu->ip += 4;
				printf("%p", addr);
			}
			else{ //register only
				printf("%s", reg_names_32[RM(modrm)]); //do nothing, we already got the reg
			}
			break;
		case 1: //disp8
			addr += (int32_t)*(int8_t*)disp;
			cpu->ip++;
			printf("+%p", (int32_t)*(int8_t*)disp);
			break;
		case 2: //disp32
			addr += *disp;
			cpu->ip += 4;
			printf("+%p", *disp);
			break;
	}

	printf("]");

	return addr;
}

uint8_t* virtual_to_physical_addr(i386* cpu, uint32_t vaddr){
	uint32_t pde = GET_PDE(vaddr);
	uint32_t pte = GET_PTE(vaddr);

	if (cpu->page_dir.entries[pde] == 0 || cpu->page_dir.entries[pde]->entries[pte] == 0){
		printf("Page fault accessing address %p.\n", vaddr);
		cpu->running = 0;
		return 0;
	}

	return cpu->page_dir.entries[pde]->entries[pte] + (vaddr & 0xfff);
}

//Maps one page from physical memory into virtual memory
void virtual_mmap(i386* cpu, uint32_t vaddr, uint8_t* paddr){
	uint32_t pde = GET_PDE(vaddr);
	uint32_t pte = GET_PTE(vaddr);

	if (cpu->page_dir.entries[pde] == 0){ //the virtual address points to an invalid page table
		cpu->page_dir.entries[pde] = (i386_PT*)malloc(sizeof(i386_PT));
	}

	cpu->page_dir.entries[pde]->entries[pte] = paddr;

	printf("      Mapping 4K page from %p into %p\n", paddr, vaddr);
}

void map_section(i386* cpu, uint32_t vaddr, uint8_t* paddr, uint32_t size){
	if (size == 0) return;

	printf("    Mapping %d bytes from %p into %p\n", size, paddr, vaddr);

	//work in 4K blocks
	for (int i = 0; i < size; i += 4096){
		virtual_mmap(cpu, vaddr + i, paddr + i);
	}
}

void op_68(i386* cpu){ //push imm16/32
	uint32_t imm = *(uint32_t*)(virtual_to_physical_addr(cpu, cpu->eip + 1));

	printf("PUSH ", imm);

	switch (cpu->operand_size){
		case 0:
			printf("%04x", imm);
			cpu_push16(cpu, (uint16_t*)&imm);
			cpu->eip += 3;
			break;
		case 1:
			printf("%08x", imm);
			cpu_push32(cpu, &imm);
			cpu->eip += 5;
			break;
	}
}

void op_6A(i386* cpu){ //push imm8
	uint32_t imm = *(uint8_t*)(virtual_to_physical_addr(cpu, cpu->eip + 1));

	printf("PUSH %02x", imm);

	switch (cpu->operand_size){
		case 0:
			cpu_push16(cpu, (uint16_t*)&imm);
			break;
		case 1:
			cpu_push32(cpu, &imm);
			break;
	}

	cpu->eip += 2;
}

void op_83(i386* cpu){ //op r/m16, imm8
	get_modrm();
	printf("%s ", arith_family_names[REG(modrm)]);
	cpu->eip += 2;
	get_modrm_dst_ptr(1);

	uint32_t imm = *(uint8_t*)(virtual_to_physical_addr(cpu, cpu->eip));
	uint32_t *src_ptr = &imm;
	printf("%02x", imm);
	cpu->eip++;

	finish_op(arith_family_fns_16[REG(modrm)], arith_family_fns_32[REG(modrm)]);
}

void op_89(i386* cpu){ //MOV r/m32, r32
	printf("MOV ");

	get_modrm();
	get_modrm_src_reg_1632();
	cpu->eip += 2;
	get_modrm_dst_ptr(1);
	finish_op(cpu_mov16, cpu_mov32);

	printf("%s", tables[cpu->operand_size][REG(modrm)]);
}

void op_B8(i386* cpu){ //MOV EAX, imm32
	printf("MOV ");
	cpu->eip++;
	uint32_t imm = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->eip);

	switch (cpu->operand_size){
		case 0: //16-bit
			printf("AX, %04x", imm);
			cpu->ax = imm;
			cpu->eip += 2;
			break;
		case 1: //32-bit
			printf("EAX, %08x", imm);
			cpu->eax = imm;
			cpu->eip += 4;
			break;
	}
}

void op_C3(i386* cpu){ //ret
	printf("RET");
	cpu->eip = cpu_pop32(cpu);
}

void op_CD(i386* cpu){ //int
	uint8_t interrupt_vector = *virtual_to_physical_addr(cpu, cpu->eip + 1);
	printf("INT %02x", interrupt_vector);

	switch (interrupt_vector){
		case 0x80:
			handle_syscall(cpu);
			break;
		case 0x03:
			printf("Debug exception.");
			cpu->running = 0;
			break;
		default:
			printf("Unknown interrupt vector.");
			cpu->running = 0;
			break;
	}

	cpu->eip += 2;
}

void op_E8(i386* cpu){ //CALL rel16/rel32
	printf("CALL ");
	uint32_t imm = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->eip + 1);
	uint32_t target;

	switch (cpu->operand_size){
		case 0:
			cpu->eip += 3;
			imm = (int32_t)(int16_t)imm;
			break;
		case 1:
			cpu->eip += 5;
			break;
	}

	target = cpu->eip + imm;

	printf("%p", target);
	cpu_call(cpu, &target);
}

void op_F4(i386* cpu){
	printf("HLT");
	cpu->eip++;
	cpu->running = 0;
}

void op_FF(i386* cpu){ //well this one does just about everything
	get_modrm();
	cpu->eip += 2;
	printf("%s ", ff_family_names[REG(modrm)]);
	get_modrm_dst_ptr(0);
	switch (cpu->operand_size){
		case 0:
			ff_family_fns_16[REG(modrm)](cpu, (uint16_t*)dst_ptr);
			break;
		case 1:
			ff_family_fns_32[REG(modrm)](cpu, dst_ptr);
			break;
	}
	
}

void(*op_table[256])(i386* cpu) = {
	0, //0x0
	0, //0x1
	0, //0x2
	0, //0x3
	0, //0x4
	0, //0x5
	0, //0x6
	0, //0x7
	0, //0x8
	0, //0x9
	0, //0xa
	0, //0xb
	0, //0xc
	0, //0xd
	0, //0xe
	0, //0xf
	0, //0x10
	0, //0x11
	0, //0x12
	0, //0x13
	0, //0x14
	0, //0x15
	0, //0x16
	0, //0x17
	0, //0x18
	0, //0x19
	0, //0x1a
	0, //0x1b
	0, //0x1c
	0, //0x1d
	0, //0x1e
	0, //0x1f
	0, //0x20
	0, //0x21
	0, //0x22
	0, //0x23
	0, //0x24
	0, //0x25
	0, //0x26
	0, //0x27
	0, //0x28
	0, //0x29
	0, //0x2a
	0, //0x2b
	0, //0x2c
	0, //0x2d
	0, //0x2e
	0, //0x2f
	0, //0x30
	0, //0x31
	0, //0x32
	0, //0x33
	0, //0x34
	0, //0x35
	0, //0x36
	0, //0x37
	0, //0x38
	0, //0x39
	0, //0x3a
	0, //0x3b
	0, //0x3c
	0, //0x3d
	0, //0x3e
	0, //0x3f
	0, //0x40
	0, //0x41
	0, //0x42
	0, //0x43
	0, //0x44
	0, //0x45
	0, //0x46
	0, //0x47
	0, //0x48
	0, //0x49
	0, //0x4a
	0, //0x4b
	0, //0x4c
	0, //0x4d
	0, //0x4e
	0, //0x4f
	0, //0x50
	0, //0x51
	0, //0x52
	0, //0x53
	0, //0x54
	0, //0x55
	0, //0x56
	0, //0x57
	0, //0x58
	0, //0x59
	0, //0x5a
	0, //0x5b
	0, //0x5c
	0, //0x5d
	0, //0x5e
	0, //0x5f
	0, //0x60
	0, //0x61
	0, //0x62
	0, //0x63
	0, //0x64
	0, //0x65
	0, //0x66
	0, //0x67
	op_68, //0x68
	0, //0x69
	op_6A, //0x6a
	0, //0x6b
	0, //0x6c
	0, //0x6d
	0, //0x6e
	0, //0x6f
	0, //0x70
	0, //0x71
	0, //0x72
	0, //0x73
	0, //0x74
	0, //0x75
	0, //0x76
	0, //0x77
	0, //0x78
	0, //0x79
	0, //0x7a
	0, //0x7b
	0, //0x7c
	0, //0x7d
	0, //0x7e
	0, //0x7f
	0, //0x80
	0, //0x81
	0, //0x82
	op_83, //0x83
	0, //0x84
	0, //0x85
	0, //0x86
	0, //0x87
	0, //0x88
	op_89, //0x89
	0, //0x8a
	0, //0x8b
	0, //0x8c
	0, //0x8d
	0, //0x8e
	0, //0x8f
	0, //0x90
	0, //0x91
	0, //0x92
	0, //0x93
	0, //0x94
	0, //0x95
	0, //0x96
	0, //0x97
	0, //0x98
	0, //0x99
	0, //0x9a
	0, //0x9b
	0, //0x9c
	0, //0x9d
	0, //0x9e
	0, //0x9f
	0, //0xa0
	0, //0xa1
	0, //0xa2
	0, //0xa3
	0, //0xa4
	0, //0xa5
	0, //0xa6
	0, //0xa7
	0, //0xa8
	0, //0xa9
	0, //0xaa
	0, //0xab
	0, //0xac
	0, //0xad
	0, //0xae
	0, //0xaf
	0, //0xb0
	0, //0xb1
	0, //0xb2
	0, //0xb3
	0, //0xb4
	0, //0xb5
	0, //0xb6
	0, //0xb7
	op_B8, //0xb8
	0, //0xb9
	0, //0xba
	0, //0xbb
	0, //0xbc
	0, //0xbd
	0, //0xbe
	0, //0xbf
	0, //0xc0
	0, //0xc1
	0, //0xc2
	op_C3, //0xc3
	0, //0xc4
	0, //0xc5
	0, //0xc6
	0, //0xc7
	0, //0xc8
	0, //0xc9
	0, //0xca
	0, //0xcb
	0, //0xcc
	op_CD, //0xcd
	0, //0xce
	0, //0xcf
	0, //0xd0
	0, //0xd1
	0, //0xd2
	0, //0xd3
	0, //0xd4
	0, //0xd5
	0, //0xd6
	0, //0xd7
	0, //0xd8
	0, //0xd9
	0, //0xda
	0, //0xdb
	0, //0xdc
	0, //0xdd
	0, //0xde
	0, //0xdf
	0, //0xe0
	0, //0xe1
	0, //0xe2
	0, //0xe3
	0, //0xe4
	0, //0xe5
	0, //0xe6
	0, //0xe7
	op_E8, //0xe8
	0, //0xe9
	0, //0xea
	0, //0xeb
	0, //0xec
	0, //0xed
	0, //0xee
	0, //0xef
	0, //0xf0
	0, //0xf1
	0, //0xf2
	0, //0xf3
	op_F4, //0xf4
	0, //0xf5
	0, //0xf6
	0, //0xf7
	0, //0xf8
	0, //0xf9
	0, //0xfa
	0, //0xfb
	0, //0xfc
	0, //0xfd
	0, //0xfe
	op_FF, //0xff
};

void cpu_dump(i386* cpu){
	printf("EAX=%08x EBX=%08x ECX=%08x EDX=%08x EFLAGS=%08x\n", cpu->eax, cpu->ebx, cpu->ecx, cpu->edx, cpu->eflags);
	printf("ESI=%08x EDI=%08x ESP=%08x EBP=%08x EIP=%08x\n", cpu->esi, cpu->edi, cpu->esp, cpu->ebp, cpu->eip);
}

void cpu_init(i386* cpu){
	memset(cpu, 0, sizeof(i386));
	cpu->running = 1;
	cpu->operand_size = 1;
	cpu->esp = STACK_BASE;
	memset(&(cpu->page_dir), 0, sizeof(i386_PD));
}

void cpu_step(i386* cpu){
 	uint8_t byte = *(virtual_to_physical_addr(cpu, cpu->eip));
	printf("%p: ", cpu->eip);

	if (op_table[byte] == 0){
		printf("Unimplemented instruction %x at %08x\n", byte, cpu->eip);
		cpu->running = 0;
	}
	else{
		op_table[byte](cpu);
	}

	printf("\n");
}