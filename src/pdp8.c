#include <pdp8.h>
#include <stdint.h>
#include <stdio.h>

void print_ram(word_t* ram, uint16_t from, uint16_t to, bool no_empty)
{
	PRE_CONDITION(from <= PDP8_MEMORY_SIZE, "`from` (%d | 0x%03X) is bigger than `PDP8_MEMORY_SIZE` (%d | 0x%03X)", from, from, PDP8_MEMORY_SIZE, PDP8_MEMORY_SIZE);
	PRE_CONDITION(to < PDP8_MEMORY_SIZE, "`to` (%d | 0x%03X) is bigger than `PDP8_MEMORY_SIZE` (%d | 0x%03X)", to, to, PDP8_MEMORY_SIZE, PDP8_MEMORY_SIZE);
	PRE_CONDITION(from <= to, "`from` (%d | 0x%03X) is equal or bigger than `to` (%d | 0x%03X)", from, from, to, to);

	printf("Dec\t");
	printf("Hex\t");
	printf("Address value\n\n");

	for (uint16_t i = from; i <= to; i++)
	{
		if (no_empty && !ram[i]) continue;

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

void execute_cycle(PDP8* cpu, bool peculiar_fmt)
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
			if (cpu->accumulator == UINT16_MAX) cpu->e++;

			cpu->accumulator++;

			break;

		case SPA:
			if (cpu->accumulator >= 0) cpu->pc++;

			break;

		case SNA:
			if (cpu->accumulator < 0) cpu->pc++;

			break;

		case SZA:
			if (cpu->accumulator == 0) cpu->pc++;

			break;

		case SZE:
			if (cpu->e == 0) cpu->pc++;

			break;

		case HLT:
			cpu->s = 0;

			break;

		case INP:
			if (peculiar_fmt)
				printf("\n\n");

			printf("INP: ");

			cpu->accumulator = getchar();

			// Clean the stdin
			while (getchar() != '\n' && !feof(stdin));

			if (peculiar_fmt)
				printf("\n\n");

			break;

		case OUT:
			if (peculiar_fmt)
				printf("\n\n");

			printf("OUT: %d   0b%016b   0x%X\n", cpu->accumulator, cpu->accumulator, cpu->accumulator);

			if (peculiar_fmt)
				printf("\n\n");

			break;
	}
}

void interrupt_cycle(PDP8* cpu)
{
	UNIMPLEMENTED;
}
