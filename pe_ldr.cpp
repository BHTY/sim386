// pe_ldr.cpp : Defines the entry point for the console application.
//

/*
	Program Procedure
	The PE loader module loads the relevant PE image and calls the CPU functions to map the relevant data into its address space


*/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "headers.h"
#include "../sim386.h"

uint32_t thunk_MessageBoxA(i386* cpu){
	HWND hwnd;
	uint32_t lpText;
	uint32_t lpCaption;
	uint32_t uType;

	hwnd = (HWND)*(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);//NULL;
	lpText = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);//0x401020;
	lpCaption = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);//0x401020;
	uType = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);//0;

	printf("\nCalling MessageBoxA(%08x, %p, %p, %08x)", hwnd, lpText, lpCaption, uType);

	return MessageBoxA(hwnd, (LPCSTR)virtual_to_physical_addr(cpu, lpText), (LPCSTR)virtual_to_physical_addr(cpu, lpCaption), uType);
}

uint32_t thunk_ExitProcess(i386* cpu){
	uint32_t uExitCode = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);

	printf("\nCalling ExitProcess(%08x)", uExitCode);

	return 0;
}

uint32_t(*thunk_table[256])(i386*) = { thunk_MessageBoxA, thunk_ExitProcess };

void handle_syscall(i386* cpu){
	int function_id = cpu->eax;

	if (thunk_table[function_id] == 0){
		printf("Unimplemented thunk %d!\n");
		cpu->running = 0;
	}
	else{
		cpu->eax = thunk_table[function_id](cpu);
	}
}

TAKEN_BASE* taken_bases = 0;

int is_base_taken(uint32_t base){
	TAKEN_BASE* temp = taken_bases;

	while (temp){
		if (temp->base == base) return 1;

		temp = temp->next;
	}

	return 0;
}

uint32_t find_free_base(uint32_t base){
	while (is_base_taken(base)){
		base += 0x400000;
	}

	return base;
}

LOADED_PE_IMAGE load_pe_file(const char* filename){
	LOADED_PE_IMAGE image;
	size_t sz;
	FILE* fp = fopen(filename, "rb");
	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);
	image.data = (BYTE*)malloc(sz);
	fseek(fp, 0, SEEK_SET);
	fread(image.data, 1, sz, fp);
	fclose(fp);

	printf("Loading PE image %s\n", filename);
	image.import_table = 0;
	image.export_table = 0;

	return image;
}

/*
Returns failure if the MZ or PE headers cannot be found.
*/

int load_pe_header(LOADED_PE_IMAGE* image){
	IMAGE_DOS_HEADER* mz_header = (IMAGE_DOS_HEADER*)image->data;
	image->nt_headers = (IMAGE_NT_HEADERS*)(image->data + mz_header->e_lfanew);

	return 0;
}

void parse_optional_header(LOADED_PE_IMAGE* image, i386* cpu){
	_IMAGE_OPTIONAL_HEADER* optional_header = &(image->nt_headers->OptionalHeader);	

	if (taken_bases == 0){
		image->image_base = optional_header->ImageBase;
		taken_bases = (TAKEN_BASE*)malloc(sizeof(TAKEN_BASE));
		taken_bases->base = image->image_base;
		taken_bases->next = 0;
	}
	else{
		image->image_base = find_free_base(optional_header->ImageBase);
		TAKEN_BASE* temp = taken_bases;
		taken_bases = (TAKEN_BASE*)malloc(sizeof(TAKEN_BASE));
		taken_bases->base = image->image_base;
		taken_bases->next = temp;
	}
	
	printf("  Image Base: %p\n", image->image_base);
	printf("  Stack Size: %d bytes committed (%d reserved)\n", optional_header->SizeOfStackCommit, optional_header->SizeOfStackReserve);

	//allocate a stack if none exists
	uint32_t stack_pde = GET_PDE(STACK_BASE - 0x1000);
	uint32_t stack_pte = GET_PTE(STACK_BASE - 0x1000);

	if (cpu->page_dir.entries[stack_pde] == 0){
		cpu->page_dir.entries[stack_pde] = (i386_PT*)malloc(sizeof(i386_PT));
		memset(cpu->page_dir.entries[stack_pde], 0, sizeof(i386_PT));
	}

	if (cpu->page_dir.entries[stack_pde]->entries[stack_pte] == 0){
		virtual_mmap(cpu, STACK_BASE - 0x1000, (uint8_t*)malloc(0x1000));
	}
}

void parse_data_directories(LOADED_PE_IMAGE* image, i386* cpu){
	IMAGE_DATA_DIRECTORY* data_dir;
	IMAGE_OPTIONAL_HEADER *optional_header = &(image->nt_headers->OptionalHeader);

	//parse export directory -- this one's important
	DWORD* exportAddressTable;
	WORD* nameOrdinalsPointer;
	DWORD* exportNamePointerTable;
	IMAGE_EXPORT_DIRECTORY* imageExportDirectory;
	EXPORTED_FUNCTION* temp;
	data_dir = &(optional_header->DataDirectory[0]);

	if (data_dir->Size != 0){
		imageExportDirectory = (IMAGE_EXPORT_DIRECTORY*)(virtual_to_physical_addr(cpu, data_dir->VirtualAddress + image->image_base));

		exportAddressTable = (DWORD*)virtual_to_physical_addr(cpu, image->image_base + imageExportDirectory->AddressOfFunctions);
		nameOrdinalsPointer = (WORD*)virtual_to_physical_addr(cpu, image->image_base + imageExportDirectory->AddressOfNameOrdinals);
		exportNamePointerTable = (DWORD*)virtual_to_physical_addr(cpu, image->image_base + imageExportDirectory->AddressOfNames);

		printf("  Export Table Size: %d bytes (%d entries)\n", data_dir->Size, imageExportDirectory->NumberOfNames);

		for (int nameIndex = 0; nameIndex < imageExportDirectory->NumberOfNames; nameIndex++){
			temp = image->export_table;
			image->export_table = (EXPORTED_FUNCTION*)malloc(sizeof(EXPORTED_FUNCTION));
			memset(image->export_table, 0, sizeof(image->export_table));

			image->export_table->ordinal = nameOrdinalsPointer[nameIndex];
			image->export_table->next = temp;
			image->export_table->address = image->image_base + exportAddressTable[image->export_table->ordinal];
			image->export_table->name = image->image_base + exportNamePointerTable[nameIndex];

			printf("    %p: %s (ordinal %d)\n", image->export_table->address, virtual_to_physical_addr(cpu, image->export_table->name), image->export_table->ordinal);
		}
	}

	//parse import directory -- this one's important
	//parse resource directory
	//parse exception directory
	//parse security directory
	//parse base relocation table
	//parse debug directory
	//parse arch specific data
	//parse rva of gp
	//parse tls directory
	//parse load config dir
	//parse bound import dir in headers
	//parse import address table -- seems pretty important to me
	//parse delay load import descriptors
	//parse com runtime descriptor
}

/*
	This is the function that loads each DLL and puts the address of any imported functions into the relevant area
*/

void parse_ilt(LOADED_PE_IMAGE* image, uint32_t* ilt, uint32_t rva_ilt, BYTE* addr_base, i386* cpu){
	IMPORTED_FUNCTION* temp;
	IMAGE_IMPORT_BY_NAME *name_table;

	while (1){
		if (*ilt == 0){
			break;
		}

		temp = image->import_table->fn_table;
		image->import_table->fn_table = (IMPORTED_FUNCTION*)malloc(sizeof(IMPORTED_FUNCTION));
		memset(image->import_table->fn_table, 0, sizeof(IMPORTED_FUNCTION));
		image->import_table->fn_table->next = temp;
		image->import_table->fn_table->address = rva_ilt;

		if ((*ilt) & 0x80000000){ //imported by ordinal
			printf("      (%x) Function imported by ordinal %d.\n", rva_ilt, (*ilt) & 0xFFFF);
			image->import_table->fn_table->ordinal_or_name = 0;
			image->import_table->fn_table->ordinal = (*ilt) & 0xFFFF;
		}else{ //imported by name
			name_table = (IMAGE_IMPORT_BY_NAME*)(addr_base + ((*ilt) & 0x7FFFFFFF));
			printf("      (%x) Function imported by name %s.\n", rva_ilt, name_table->Name);
			image->import_table->fn_table->ordinal_or_name = 1;
			image->import_table->fn_table->name = (char*)malloc(strlen(name_table->Name) + 1);
			strcpy(image->import_table->fn_table->name, name_table->Name);
		}

		ilt++;
		rva_ilt += 4;
	}
}

void parse_idt(LOADED_PE_IMAGE* image, IMAGE_SECTION_HEADER* sec, i386* cpu){
	IMPORT_TABLE* temp;
	IMAGE_IMPORT_DESCRIPTOR* import_desc = (IMAGE_IMPORT_DESCRIPTOR*)(image->data + sec->PointerToRawData);
	IMAGE_IMPORT_DESCRIPTOR null_desc;
	BYTE* address_base = image->data + sec->PointerToRawData - sec->VirtualAddress;
	memset(&null_desc, 0, sizeof(null_desc));

	while (1){
		//check if it's zeroed out - if it is, quit the loop
		if (memcmp(import_desc, &null_desc, sizeof(IMAGE_IMPORT_DESCRIPTOR)) == 0){
			//printf("    Done parsing IDT\n");
			break;
		}

		printf("    IAT RVA = %x, ILT RVA = %x ", import_desc->OriginalFirstThunk, import_desc->FirstThunk);
		printf("Name: %s\n", address_base + import_desc->Name);

		temp = image->import_table;
		image->import_table = (IMPORT_TABLE*)malloc(sizeof(IMPORT_TABLE));
		memset(image->import_table, 0, sizeof(IMPORT_TABLE));
		image->import_table->image_name = address_base + import_desc->Name;
		image->import_table->next = temp;

		//iterate over IAT & load binaries
		parse_ilt(image, (uint32_t*)(address_base + import_desc->OriginalFirstThunk), import_desc->FirstThunk, address_base, cpu);

		import_desc++;
	}
}

void parse_rt(LOADED_PE_IMAGE* image, IMAGE_SECTION_HEADER* sec, i386* cpu){
	IMAGE_BASE_RELOCATION* reloc = (IMAGE_BASE_RELOCATION*)(image->data + sec->PointerToRawData);
	IMAGE_BASE_RELOCATION null_reloc;
	uint32_t num_entries;
	uint16_t reloc_entry;

	memset(&null_reloc, 0, sizeof(IMAGE_BASE_RELOCATION));

	while (1){
		if (memcmp(reloc, &null_reloc, sizeof(IMAGE_BASE_RELOCATION)) == 0){
			//printf("    Done parsing relocations table\n");
			break;
		}

		num_entries = (reloc->SizeOfBlock - 8) / 2;

		for (int i = 0; i < num_entries; i++){
			reloc_entry = *((WORD*)((BYTE*)reloc + sizeof(IMAGE_BASE_RELOCATION)) + i);
			//printf("    Relocation entry %04x from %p\n", reloc_entry, reloc->VirtualAddress);
		}

		reloc = (IMAGE_BASE_RELOCATION*)((BYTE*)reloc + reloc->SizeOfBlock);
	}
}

void parse_text(LOADED_PE_IMAGE* image, IMAGE_SECTION_HEADER* sec, i386* cpu){
	virtual_mmap(cpu, sec->VirtualAddress + image->image_base, image->data + sec->PointerToRawData); //temporary hack!

	if (cpu->eip == 0){
		cpu->eip = sec->VirtualAddress + image->image_base;
	}
}

void parse_section_headers(LOADED_PE_IMAGE* image, i386* cpu){
	IMAGE_SECTION_HEADER current_section; //reloc_pointer & num_relocs don't matter for EXEs
	image->section_headers = (IMAGE_SECTION_HEADER*)((BYTE*)&(image->nt_headers->OptionalHeader) + image->nt_headers->FileHeader.SizeOfOptionalHeader);

	for (int i = 0; i < image->nt_headers->FileHeader.NumberOfSections; i++){
		current_section = image->section_headers[i];
		printf("  Section %s\n", current_section.Name);
		printf("    Physical Address: %p Virtual Address %p Size: %d\n", current_section.PointerToRawData, current_section.VirtualAddress, current_section.SizeOfRawData);

		if (strcmp((const char*)current_section.Name, ".text") == 0){
			parse_text(image, &current_section, cpu);
		} else if(strcmp((const char*)current_section.Name, ".idata") == 0){
			parse_idt(image, &current_section, cpu);
		} else if (strcmp((const char*)current_section.Name, ".reloc") == 0){
			parse_rt(image, &current_section, cpu);
		}

		map_section(cpu, current_section.VirtualAddress + image->image_base, image->data + current_section.PointerToRawData, current_section.SizeOfRawData);
	}
}

void resolve_imports(LOADED_PE_IMAGE*, i386*);		

void parse_headers(LOADED_PE_IMAGE* image, i386* cpu){
	load_pe_header(image);
	if (image->nt_headers->FileHeader.SizeOfOptionalHeader != 0){
		parse_optional_header(image, cpu);
	}
	parse_section_headers(image, cpu);
	parse_data_directories(image, cpu);
	resolve_imports(image, cpu);
}

void print_export_table(LOADED_PE_IMAGE* image, i386* cpu){
	printf("EXPORT TABLE\n");
	EXPORTED_FUNCTION* table = image->export_table;

	while (table){
		printf("  %p: %s (ordinal %d)\n", table->address, virtual_to_physical_addr(cpu, table->name), table->ordinal);
		table = table->next;
	}
}

uint32_t scan_export_table_for_name(EXPORTED_FUNCTION* export_table, i386* cpu, char* name){
	while (export_table){
		if (strcmp(name, (const char*)virtual_to_physical_addr(cpu, export_table->name)) == 0){
			return export_table->address;
		}

		export_table = export_table->next;
	}
}

uint32_t scan_export_table_for_ordinal(EXPORTED_FUNCTION* export_table, WORD ordinal){
	return 0;
}

void resolve_imports(LOADED_PE_IMAGE* image, i386* cpu){
	//do a list of all imports
	IMPORT_TABLE *table = image->import_table;
	IMPORTED_FUNCTION *function;
	LOADED_PE_IMAGE new_image;
	uint32_t addr;
	uint32_t* paddr;

	while (table){
		printf("IMPORTS FROM %s\n", table->image_name);
		new_image = load_pe_file((const char*)(table->image_name));
		parse_headers(&new_image, cpu);

		function = table->fn_table;

		//load the relevant PE image and then link your import table with their export table

		while (function){
			printf("  (%p) ", image->image_base + function->address);

			if (function->ordinal_or_name){
				printf("%s", function->name);
				addr = scan_export_table_for_name(new_image.export_table, cpu, function->name);
			}
			else{
				printf("%d", function->ordinal);
			}

			printf(" (resolved to address %p)\n", addr);

			*(uint32_t*)(virtual_to_physical_addr(cpu, image->image_base + function->address)) = addr;

			function = function->next;
		}

		table = table->next;
	}
}

int main(int argc, char* argv[])
{
	i386 CPU;
	cpu_init(&CPU);
	LOADED_PE_IMAGE hello = load_pe_file("C:\\Users\\Will\\peldr\\hello.exe");

	parse_headers(&hello, &CPU);

	while (CPU.running){
		cpu_step(&CPU);
	}

	cpu_dump(&CPU);

	getchar();

	return 0;
}

