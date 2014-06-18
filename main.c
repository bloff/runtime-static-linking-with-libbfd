#define PACKAGE "circe-dynload-test"
#define PACKAGE_VERSION "0.1"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
// #include <config.h>
#include <bfd.h>




////////////////////////////////////////////////////////////////////////////////
//  change these a little bit for different behavior
//
////////////////////////////////////////////////////////////////////////////////

//// callbacks


// a function to be used as callback
int my_callback_01(int a) {
	printf("my_callback_01 called!\n");
	return a*2;
}

int my_callback_02(int a) {
	printf("my_callback_02 called!\n");
	return a*4;
}

typedef int (*t_callback)(int);
typedef void (*t_test_function)(int, int*);


// test_unit.o is expected to call a function with the name "callback";
// we will relocate those calls to the address in the my_callback variable
t_callback my_callback = my_callback_02;
// our job is to load the binary code of the object file into memory,
// then find the address of the function with the following name
const char *test_function_name = "test_function_02";
// store it on the following pointer:
// and then execute it on the two arguments "in" and "out":
int in = 10;
int out[2] = {0,0};

// this pointer will eventually store the address of the function in test_unit.o with the name test_function_name
t_test_function test_function;










typedef void *ptr_t;
typedef unsigned char byte_t;

static byte_t *memory = NULL;
static size_t size = 0;

static size_t pagesize;


// Returns the first number greater or equal than "value" which is a multiple of "size"
size_t _round_up_to_multiple_of(size_t value, size_t size) {
  size_t r = value % size;
  return r == 0 ? value : value - r + size;
}

// allocs an aligned block of at least minMemSize bytes of zeroed memory,
// which is readable, writable and executable
void alloc_rwx(size_t minMemSize) {
  // TODO: make sure this works for windows and not just POSIX
  
  size_t pagesize = sysconf(_SC_PAGESIZE);
  
  if (minMemSize <= pagesize)
    size = pagesize;
  else
    size = _round_up_to_multiple_of(size, pagesize);
    
  if (posix_memalign((void**)&memory, pagesize, size) != 0)
    exit(1); // TODO: error msg
  if (mprotect(memory, size, PROT_READ | PROT_WRITE | PROT_EXEC) != 0)
    exit(1); // TODO: error msg
  memset(memory, 0, size);
}







int main(int argc, char **argv) {
		
	// init libbfd
	bfd_init();
	
    printf("loading test_unit.o.\n");
	
	// load test_unit.o object file
	bfd *abfd = bfd_openr("test_unit.o", NULL);

	// no section info is loaded unless we call bfd_check_format!:
	if (!bfd_check_format (abfd, bfd_object)) {
		printf("Failed to open object file!\n");
		exit(-1);        
    }
	
	
	// load .text (binary code) and .data (initialized data);
	// eventually need to load .bss also (uninitialized data).
	asection *text = bfd_get_section_by_name (abfd, ".text");
	asection *data = bfd_get_section_by_name (abfd, ".data");
	
	
	// we need enough memory to store both data and binary code
	size_t minMemSize = data->size + text->size;
	
	// allocate that much memory, which is both readable, writable and executable
	alloc_rwx(minMemSize);
	
	// copy the contents of the data and executable sections into the newly allocated memory
	bfd_get_section_contents(abfd, data, memory, 0, data->size);
	bfd_get_section_contents(abfd, text, memory + data->size, 0, text->size);
	
	// store the memory addresses where the sections start
	void *data_start = memory;
	void *text_start = memory + data->size;
	
	// in order to perform relocations, bfd must know what these addresses are
	// this is how we let it know
	text->output_offset = (long unsigned int) text_start;
	data->output_offset = (long unsigned int) data_start;
	
	
	// load the symbol table from the object file
	size_t symsize = bfd_get_symtab_upper_bound(abfd);
	asymbol **symbols = malloc(symsize);
	int symcount = bfd_canonicalize_symtab(abfd, symbols);
	
	int i;
	// we look for two special symbols
	for (i = 0; i<symcount; i++) {
		// "callback" should appear on the special "undefined" section of the
		// object file; this section should have starting address = 0, and hence
		// we set the value of the symbol to be the address of our callback
		if (!strcmp(symbols[i]->name, "callback")) {
			printf("Changing address for undefined symbol: %s\n", symbols[i]->name);
			symbols[i]->value = (long unsigned int) my_callback;
		}
		// test_function_name should be defined somewhere in the .text section,
		// and the value of that symbol should be the entry point for the corresponding
		// function relative to the start of the .text section;
		// since we copied the .text section to memory starting at the text_start address,
		// the true entry point will be text_start + value of that symbol
		if (!strcmp(symbols[i]->name, test_function_name)) {
			printf("Storing address of symbol: %s!\n", symbols[i]->name);
			test_function = text_start + symbols[i]->value;
		}
	}
	
	// Now we load the relocation table
	long relsize = bfd_get_reloc_upper_bound (abfd, text);
	arelent **relpp = malloc (relsize);
	long relcount = bfd_canonicalize_reloc (abfd, text, relpp, symbols);
	char *tmp;
	
	
	
	
	for (i = 0; i<relcount; i++) {
		arelent *reloc = relpp[i];
		// we want to look for every call to the symbol "callback",
		// and relocate ("patch") that call so that it calls the function
		// whose pointer we stored in the "callback" symbol entry
		// (which in turn was the contents of the my_callback pointer)
		if (!strcmp((*reloc->sym_ptr_ptr)->name, "callback")) {
			printf("Found relocation for symbol: %s!\n", (*reloc->sym_ptr_ptr)->name);
			text->output_section = (*reloc->sym_ptr_ptr)->section;
			bfd_perform_relocation(abfd, reloc, text_start, text, NULL, &tmp);
		}
	}
	
	
	// once everything is patched, we should be able to run test_function
	// which should call our callback!
	
	test_function(in,out);
	
	printf("calling \"%s\" from test_unit.o on %d, and \"out\".\n", test_function_name, in);
	printf("out = { %d, %d }\n", out[0], out[1]);
	
	free(symbols);
	free (relpp);
	bfd_close(abfd);
    return 0;
}


