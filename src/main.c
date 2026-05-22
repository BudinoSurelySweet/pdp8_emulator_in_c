#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Number of word available inside the RAM of this version of PDP8.
#define MEMORY_SIZE 4096

// Build a MRI instruction with `i` as first bit, `opr` as 2-4th bits, and `addr` as 5-16th bits.
#define MAKE_MRI_INSTRUCTION(i, opr, addr) \
    ( (uint16_t)( (((i) & 0x1) << 15) | (((opr) & 0x7) << 12) | ((addr) & 0xFFF) ) )

// Define a pre condition contract for the parameters of a function.
#define PRE_CONDITION(condition, format, ...) \
    do { \
        if (!(condition)) \
        { \
            printf("\033[31m" "[PDP8] Contract Violation (pre-condition failed) at %s:%d\nNote: " format "\033[39m\n", \
                   __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
            exit(1); \
        } \
    } while(0)

typedef uint16_t word_t;

// This struct define a PDP8 cpu with all of it's components
typedef struct
{
	word_t memory_buffer_register;
	uint16_t memory_address_register : 12;
	uint16_t i : 1;
	uint16_t operation_code : 3;
	word_t accumulator;
	uint16_t e : 1;
	uint16_t program_counter : 12;
	uint16_t s : 1;
	uint16_t f : 1;
	uint16_t r : 1;
	word_t ram[MEMORY_SIZE];
} PDP8;

// The instruction set available inside thid version of PDP8
typedef enum : uint16_t
{
	// MRI: Memory Reference Instruction
	AND = 0b000,
	ADD = 0b001,
	LDA = 0b010,
	STA = 0b011,
	BUN = 0b100,
	BSA = 0b101,
	ISZ = 0b110,

	// RRI: Register Reference Instruction
	CLA = 0b0'111'1000'0000'0000,
	CLE = 0b0'111'0100'0000'0000,
	CMA = 0b0'111'0010'0000'0000,
	CME = 0b0'111'0001'0000'0000,
	CIR = 0b0'111'0000'1000'0000,
	CIL = 0b0'111'0000'0100'0000,
	INC = 0b0'111'0000'0010'0000,
	SPA = 0b0'111'0000'0001'0000,
	SNA = 0b0'111'0000'0000'1000,
	SZA = 0b0'111'0000'0000'0100,
	SZE = 0b0'111'0000'0000'0010,
	HLT = 0b0'111'0000'0000'0001,

	// I/O: Input / Output
	INP = 0b1'111'1000'0000'0000,
	OUT = 0b1'111'0100'0000'0000,
} instruction_set;

// Helper function that print a porton of the `ram`. This portion is defined by `from` and `to`.
//
// Contracts:
// `from` <= `MEMORY_SIZE`
// `to` <= `MEMORY_SIZE`
// `from` <= `to`
void print_ram(word_t* ram, uint16_t from, uint16_t to)
{
	PRE_CONDITION(from <= MEMORY_SIZE, "`from` (%d | 0x%03X) is bigger than `MEMORY_SIZE` (%d | 0x%03X)", from, from, MEMORY_SIZE, MEMORY_SIZE);
	PRE_CONDITION(to <= MEMORY_SIZE, "`to` (%d | 0x%03X) is bigger than `MEMORY_SIZE` (%d | 0x%03X)", to, to, MEMORY_SIZE, MEMORY_SIZE);
	PRE_CONDITION(from <= to, "`from` (%d | 0x%03X) is bigger than `to` (%d | 0x%03X)", from, from, to, to);

	printf("Dec\t");
	printf("Hex\t");
	printf("Address value\n\n");

	for (uint16_t i = from; i <= to; i++)
	{
		printf("%d\t", i);
		printf("%X\t", i);

		printf("%01b ", ram[i] >> 15);
		printf("%03b ", (ram[i] >> 12) & 0b0111);
		printf("%012b\n", (ram[i]) & 0x0fff);
	}
}

int main()
{
	PDP8 cpu = {0};

	// TODO: 1. Open file to get source code

	// TODO: 1.1 Lexer

	// TODO: 1.2 Token -> Binary

	// TODO: 2. Insert file inside RAM
	cpu.ram[0x100] = MAKE_MRI_INSTRUCTION(0, LDA, 2);
	cpu.ram[0x101] = MAKE_MRI_INSTRUCTION(0, ADD, -2);
	cpu.ram[0x102] = SZA;
	cpu.ram[0x103] = MAKE_MRI_INSTRUCTION(0, STA, 1);
	cpu.ram[0x104] = OUT;

	print_ram(cpu.ram, 0x100, 0x10a);

	// TODO: 3. PDP8.program_counter = "ORG 100"
	cpu.program_counter = 0x100;

	// TODO: 4. Cycle manager (fetch, execute, indirizzamento, interrupt)

	return 0;
}

