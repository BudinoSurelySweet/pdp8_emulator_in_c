#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Number of word available inside the RAM of this version of PDP8.
#define MEMORY_SIZE 4096

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

void fetch_cycle(PDP8* cpu)
{
	cpu->mar = cpu->pc;
	cpu->mbr = cpu->ram[cpu->mar];
	cpu->pc++;
	cpu->opr = cpu->mbr >> 12;
	cpu->i = cpu->mbr >> 15;

	if (cpu->i == 1 && cpu->opr != 0b111) cpu->r = 1;
	else cpu->f = 1;
}

void indirection_cycle(PDP8* cpu)
{
	cpu->mar = cpu->mbr;
	cpu->mbr = cpu->ram[cpu->mar];

	cpu->f = 1;
	cpu->r = 0;
}

void execute_cycle(PDP8* cpu)
{
	cpu->f = 0;

	// MRI
	if (cpu->opr != 0b111)
	{
		cpu->mar = cpu->mbr;

		switch (cpu->opr)
		{
			case AND:
				cpu->mbr = cpu->ram[cpu->mar];
				cpu->accumulator &= cpu->mbr;
				
				break;

			case ADD:
				cpu->mbr = cpu->ram[cpu->mar];

				uint32_t tmp = (uint32_t) cpu->accumulator + (uint32_t) cpu->mbr; 

				cpu->e = (tmp >> 16) & 0x1;
				cpu->accumulator = tmp & 0xFFFF;

				break;

			case LDA:
				cpu->mbr = cpu->ram[cpu->mar];
				cpu->accumulator = 0;
				cpu->accumulator += cpu->mbr;

				break;

			case STA:
				cpu->mbr = cpu->accumulator;
				cpu->ram[cpu->mar] = cpu->mbr;

				break;

			case BUN:
				cpu->pc = cpu->mbr;

				break;

			case BSA:
				cpu->mbr ^= cpu->pc;
				cpu->pc ^= cpu->mbr;
				cpu->mbr ^= cpu->pc;

				cpu->ram[cpu->mar] = cpu->mbr;
				cpu->pc++;

				break;

			case ISZ:
				cpu->mbr = cpu->ram[cpu->mar];
				cpu->mbr++;
				cpu->ram[cpu->mar] = cpu->mbr;

				if (cpu->mbr == 0) cpu->pc++;

				break;

			default:
				UNIMPLEMENTED;
		}

		return;
	}

	switch (cpu->mbr)
	{
		case CLA:
			cpu->accumulator = 0;

			break;

		case CLE:
			cpu->e = 0;

			break;

		case CMA:
			cpu->accumulator = ~cpu->accumulator;

			break;

		case CME:
			cpu->e = ~cpu->e;

			break;

		case CIR:
			char lsb = cpu->accumulator & 0x0001;

			cpu->accumulator = cpu->accumulator >> 1;

			if (cpu->e == 1) cpu->accumulator = cpu->accumulator | 0x8000;

			cpu->e = lsb;

			break;

		case CIL:
			word_t old_e = cpu->e;

			if ((cpu->accumulator & 0x8000) != 0) cpu->e = 1;
			else cpu->e = 0;

			cpu->accumulator = cpu->accumulator << 1;
			cpu->accumulator = cpu->accumulator | old_e;

			break;

		case INC:
			break;

		case SPA:
			break;

		case SNA:
			break;

		case SZA:
			break;

		case SZE:
			break;

		case HLT:
			cpu->s = 0;

			break;

		case INP:
			break;

		case OUT:
			// TODO: Sostituire il printf con putchar
			printf("%016b\n", cpu->accumulator);

			break;
	}
}

void interrupt_cycle(PDP8* cpu)
{
	UNIMPLEMENTED;
}

int main()
{
	PDP8 cpu = {0};

	// TODO: 1. Open file to get source code

	// TODO: 1.1 Lexer

	// TODO: 1.2 Token -> Binary

	// TODO: 2. Insert file inside RAM

	cpu.ram[0x100] = MAKE_MRI_INSTRUCTION(0, LDA, 0x10a);
	cpu.ram[0x101] = OUT;
	cpu.ram[0x102] = CME;
	cpu.ram[0x103] = CIR;
	cpu.ram[0x104] = OUT;
	cpu.ram[0x105] = HLT;

	cpu.ram[0x10a] = 83;

	// cpu.ram[0x100] = MAKE_MRI_INSTRUCTION(0, LDA, 0x10a);
	// cpu.ram[0x101] = OUT;
	// cpu.ram[0x102] = MAKE_MRI_INSTRUCTION(0, BSA, 0x105);
	// cpu.ram[0x103] = OUT;
	// cpu.ram[0x104] = HLT;
	// cpu.ram[0x105] = 0;
	// cpu.ram[0x106] = MAKE_MRI_INSTRUCTION(0, LDA, 0x10b);
	// cpu.ram[0x107] = MAKE_MRI_INSTRUCTION(1, BUN, 0x105);
	//
	// cpu.ram[0x10a] = 1;
	// cpu.ram[0x10b] = 2;

	// TODO: 3. PDP8.program_counter = "ORG 100"
	cpu.pc = 0x100;

	// TODO: 4. Cycle manager (fetch, execute, indirizzamento, interrupt)

	cpu.s = 1;
	
	while (cpu.s)
	{
		if (cpu.f == 0 && cpu.r == 0)
		{
			fetch_cycle(&cpu);
		}
		else if (cpu.f == 0 && cpu.r == 1)
		{
			indirection_cycle(&cpu);
		}
		else if (cpu.f == 1 && cpu.r == 0)
		{
			execute_cycle(&cpu);
		}
		else
		{
			interrupt_cycle(&cpu);
		}
	}

	return 0;
}

