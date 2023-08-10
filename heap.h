#pragma once

#include <stdint.h>
#include "../sim386.h"
#include "headers.h"

#define NUM_HEAP 100

#define align(x)	(((x % 4) == 0) ? x : (x + 4) - x % 4)

typedef struct MemBlock{
	bool used;
	size_t size;
	uint32_t prev;
	uint32_t next;
	uint8_t data[0];
} MemBlock;

typedef struct{
	uint32_t exists;
	uint32_t starting_address;
	uint32_t reserved_pages;
	uint32_t committed_pages;
} HEAP;

extern HEAP heap_list[NUM_HEAP];

void heap_init();
uint32_t alloc_heap(i386* cpu, uint32_t initial_size, uint32_t max_size);
uint32_t heap_alloc(i386* cpu, uint32_t handle, uint32_t size);
uint32_t heap_realloc(i386* cpu, uint32_t handle, uint32_t addr, uint32_t new_size);
int heap_free(i386* cpu, uint32_t handle, uint32_t addr);