#ifndef PDP8_H
#define PDP8_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Number of word available inside the RAM of this version of PDP8.
#define PDP8_MEMORY_SIZE 4096

#define UNIMPLEMENTED printf("not implemented"); exit(1)

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

#define ERROR(format, ...) \
    do { \
		printf("\033[31m" "[PDP8] Error at %s:%d\nNote: " format "\033[39m\n", \
			   __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
		exit(1); \
    } while(0)

typedef uint16_t word_t;

// This struct define a PDP8 cpu with all of it's components
typedef struct
{
	word_t mbr; // Memory Buffer Register
	uint16_t mar : 12; // Memory Address Register
	uint16_t i : 1;
	uint16_t opr : 3; // Operation code
	word_t accumulator;
	uint16_t e : 1;
	uint16_t pc : 12; // Program Counter
	uint16_t s : 1;
	uint16_t f : 1;
	uint16_t r : 1;
	word_t ram[PDP8_MEMORY_SIZE];
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
void print_ram(word_t* ram, uint16_t from, uint16_t to, bool no_empty);

void fetch_cycle(PDP8* cpu);

void indirection_cycle(PDP8* cpu);

void execute_cycle(PDP8* cpu, bool peculiar_fmt);

void interrupt_cycle(PDP8* cpu);

#endif
