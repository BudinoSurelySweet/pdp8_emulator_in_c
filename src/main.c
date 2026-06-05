#include "pdp8.h"
#include <assembler.h>
#include <stdio.h>

#define ASSERT(condition, format, ...) \
    do { \
        if (!(condition)) \
        { \
            printf("\033[31m" "[PDP8] Assertion failed at %s:%d\nNote: " format "\033[39m\n", \
                   __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
            exit(1); \
        } \
    } while(0)

int main(int argc, char** argv)
{
	PDP8 cpu = {0};

	ASSERT(argc > 1, "You need to pass a path");
	cpu.pc = assemble_and_load(&cpu, argv[1]);

	// TEST: Togliere queste righe qua sotto e mettere quelle sopra
	// const char* PATH = "examples/program.pdp8";
	// cpu.pc = assemble_and_load(&cpu, PATH);

	// print_ram(cpu.ram, cpu.pc, cpu.pc + 20);

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
			printf("PC: %x\t\t", cpu.pc - 1);

			execute_cycle(&cpu);

			printf("AC: %04b %04b %04b %04b\n", (cpu.accumulator & 0xF000) >> 12, (cpu.accumulator & 0x0F00) >> 8, (cpu.accumulator & 0x00F0) >> 4, cpu.accumulator & 0x000F);
		}
		else
		{
			interrupt_cycle(&cpu);
		}
	}

	return 0;
}
