#include "pdp8.h"
#include <assembler.h>
#include <stdint.h>

#define ASSERT(condition, format, ...) \
    do { \
        if (!(condition)) \
        { \
            printf("\033[31m" "[PDP8] Assertion failed at %s:%d\nNote: " format "\033[39m\n", \
                   __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
            exit(1); \
        } \
    } while(0)

typedef struct
{
	bool show_accumulator;
	bool show_ram;
	uint16_t show_ram_from;
	uint16_t show_ram_to;
	char* program_path;
} Args;

int main(int argc, char** argv)
{
	PDP8 cpu = {0};
	Args args = {0};

	for (int i = 1; i < argc; i++)
	{
		char* flag = argv[i];
		char* param1 = i + 1 < argc ? argv[i + 1] : "No param passed";
		char* param2 = i + 1 < argc ? argv[i + 2] : "No param passed";

		if (flag[0] == '-')
		{
			if (flag[1] == 'p')
			{
				args.program_path = param1;
			}
			else if (flag[1] == 'a')
			{
				args.show_accumulator = true;
			}
			else if (flag[1] == 'r')
			{
				args.show_ram = true;
				args.show_ram_from = strtol(param1, NULL, 0);
				args.show_ram_to = strtol(param2, NULL, 0);
			}
		}
	}

	cpu.pc = assemble_and_load(&cpu, args.program_path);

	if (args.show_ram)
	{
		print_ram(cpu.ram, args.show_ram_from, args.show_ram_to, false);
		printf("\n");
	}

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
			execute_cycle(&cpu, args.show_accumulator);

			if (args.show_accumulator)
			{
				printf("PC: %x\t\t", cpu.pc);
				printf("AC: %04b %04b %04b %04b\n", (cpu.accumulator & 0xF000) >> 12, (cpu.accumulator & 0x0F00) >> 8, (cpu.accumulator & 0x00F0) >> 4, cpu.accumulator & 0x000F);
			}
		}
		else
		{
			interrupt_cycle(&cpu);
		}
	}

	return 0;
}
