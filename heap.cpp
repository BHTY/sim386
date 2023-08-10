#include "heap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

HEAP heap_list[NUM_HEAP];

void heap_init(){
	memset(heap_list, 0, sizeof(heap_list));
}

//this function creates a heap
uint32_t alloc_heap(i386* cpu, uint32_t initial_size, uint32_t max_size){
	uint32_t heap_handle = 0;
	uint32_t pages_to_reserve;
	uint32_t pages_to_commit;
	uint32_t starting_address;
	uint8_t* temp_ptr;
	MemBlock* first_block;

	for (int i = 1; i < NUM_HEAP; i++){
		if (heap_list[i].exists == 0){
			heap_handle = i;
			heap_list[i].exists = 1;
			break;
		}
	}

	if (heap_handle == 0){
		return 0;
	}

	if (max_size == 0){
		max_size = 1048576;
	}

	//otherwise, let's create a heap!
	pages_to_commit = initial_size / 4096;
	pages_to_reserve = max_size / 4096;

	if (initial_size % 4096 != 0){
		pages_to_commit++;
	}

	if (max_size % 4096 != 0){
		pages_to_reserve++;
	}

	pages_to_commit+=2;

	heap_list[heap_handle].reserved_pages = pages_to_reserve;
	heap_list[heap_handle].committed_pages = pages_to_commit;

	//now, locate a suitable block of virtual address space and reserve
	starting_address = scan_free_address_space(cpu, pages_to_reserve, 0x400);
	reserve_address_space(cpu, starting_address, pages_to_reserve);
	heap_list[heap_handle].starting_address = starting_address;

	printf("\n  Allocating %d bytes of address space from %p and mapping %d pages\n", pages_to_reserve * 0x1000, starting_address, pages_to_commit);

	//allocate the requested number of pages and map them in
	temp_ptr = (uint8_t*)malloc(pages_to_commit * 0x1000);
	map_section(cpu, starting_address, temp_ptr, pages_to_commit * 0x1000);

	//set a default value in there
	first_block = (MemBlock*)virtual_to_physical_addr(cpu, starting_address);
	first_block->used = 0;
	first_block->size = pages_to_commit * 0x1000 - sizeof(MemBlock);
	first_block->prev = 0;
	first_block->next = 0;

	return heap_handle;
}


//this function allocates FROM a heap

uint32_t heap_alloc(i386* cpu, uint32_t handle, uint32_t size){
	if (handle > NUM_HEAP || heap_list[handle].exists == 0){
		return 0;
	}

	MemBlock* curBlock;

	MemBlock* nextBlock;
	uint32_t oldNext;
	MemBlock* old_next;

	uint32_t old_size;

	uint32_t addr = heap_list[handle].starting_address;
	size = align(size);

	//scan for the first LINEAR memory block large enough to satisfy the allocation
	while (1){
		//printf("Current memory block is addr %p\n", addr);
		curBlock = (MemBlock*)virtual_to_physical_addr(cpu, addr);

		if (!(curBlock->used)){ //free block found
			if (curBlock->size >= (size + sizeof(MemBlock))){ //if the block is big enough
				old_size = curBlock->size;
				oldNext = curBlock->next;
				//format current block
				curBlock->used = 1;
				curBlock->size = size;
				curBlock->next = addr + size + sizeof(MemBlock);
				
				nextBlock = (MemBlock*)virtual_to_physical_addr(cpu, curBlock->next);
				nextBlock->used = 0;
				nextBlock->prev = addr;

				if (oldNext == 0){
					nextBlock->next = 0;
					nextBlock->size = old_size - (size + sizeof(MemBlock));
				}
				else{
					old_next = (MemBlock*)virtual_to_physical_addr(cpu, oldNext);

					if (old_next->used){
						nextBlock->next = oldNext;
						nextBlock->size = old_size - (size + sizeof(MemBlock));
					}
					else{ //oldnext is free, merge
						nextBlock->next = old_next->next;
						nextBlock->size = old_size - size + old_next->size;
					}
				}

				break;
			}
		}

		addr = curBlock->next;

		if (!addr){ //we've reached the end of the list and still found nothing big enough to satisfy the allocation
			return NULL; //grow the heap!
		}
	}

	return addr + 16;
}