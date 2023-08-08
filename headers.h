#pragma once

#include <stdint.h>

#if defined (WIN32)
#include <Windows.h>
#include <winnt.h>
#else

#error "Only works on Windows"

#endif

typedef struct TAKEN_BASE{
	uint32_t base;
	struct TAKEN_BASE* next;
} TAKEN_BASE;

typedef struct IMPORTED_FUNCTION{
	uint32_t address;
	int ordinal_or_name; //set to 0 for ordinal, 1 for name
	union{
		char* name;
		uint16_t ordinal;
	};
	struct IMPORTED_FUNCTION* next;
} IMPORTED_FUNCTION;

typedef struct IMPORT_TABLE{
	BYTE* image_name;
	IMPORTED_FUNCTION* fn_table;
	struct IMPORT_TABLE* next;
} IMPORT_TABLE;

typedef struct EXPORTED_FUNCTION{
	uint32_t address;
	uint32_t name;
	uint16_t ordinal;
	struct EXPORTED_FUNCTION* next;
} EXPORTED_FUNCTION;

typedef struct{
	BYTE* data;
	uint32_t image_base;
	IMAGE_NT_HEADERS* nt_headers;
	IMAGE_SECTION_HEADER* section_headers;
	IMPORT_TABLE* import_table;
	EXPORTED_FUNCTION *export_table;
} LOADED_PE_IMAGE;