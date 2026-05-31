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
	assemble_and_load(&cpu, argv[1]);

	// TEST: Togliere queste righe qua sotto e mettere quelle sopra
	// const char* PATH = "examples/program.pdp8";
	// cpu.pc = assemble_and_load(&cpu, PATH);

	// print_ram(cpu.ram, cpu.pc, cpu.pc + 10);

	// TODO: 2. Insert file inside RAM

	// cpu.ram[0x100] = MAKE_MRI_INSTRUCTION(0, LDA, 0x10a);
	// cpu.ram[0x101] = OUT;
	// cpu.ram[0x102] = CME;
	// cpu.ram[0x103] = CIR;
	// cpu.ram[0x104] = OUT;
	// cpu.ram[0x105] = HLT;
	//
	// cpu.ram[0x10a] = 83;

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

