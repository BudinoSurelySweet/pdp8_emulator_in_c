#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>

#include "build_system.h"

int main(int argc, char** argv)
{
	if (rebuild_self("build.conf", argv)) return 0;
	
	#include "../build.conf"

	char target_to_exec[128];
	char action[128];

	if (argc >= 2)
		strcpy(action, argv[1]);
	else
		BUILD_SYSTEM_ASSERT(false, "No message");

	if (argc >= 3)
		strcpy(target_to_exec, argv[2]);
	else
		strcpy(target_to_exec, NO_TARGET_MENTIONED);

	const char* compiler_command = generate_compiler_command(target_to_exec);

	if (
			strcmp(action, "compile") == 0
			and compiler_command
			and compiler_command[0] != '\0'
	)
	{
		BUILD_SYSTEM_COMPILE_MSG("%s", compiler_command);
		system(compiler_command);
	}
	else if (strcmp(action, "run") == 0)
	{
		BUILD_SYSTEM_COMPILE_MSG("%s", compiler_command);
		system(compiler_command);

		const char* execute_command = get_target_executable(target_to_exec);

		BUILD_SYSTEM_EXEC_MSG("%s", execute_command);
		system(execute_command);
	}

	return 0;
}

