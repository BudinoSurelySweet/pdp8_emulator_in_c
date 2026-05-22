#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define NO_TARGET_MENTIONED "__NO_TARGET_MENTIONED__"

#define BUILD_SYSTEM_ASSERT(condition, format, ...) \
    do { \
        if (!(condition)) \
        { \
            printf("\033[31m" "[Build system] Runtime assertion failed: " format "\033[39m\n" __VA_OPT__(,) __VA_ARGS__); \
            exit(1); \
        } \
    } while(0)

#define BUILD_SYSTEM_MSG(msg, ...) printf("\033[34m" "[Build system] " msg "\033[39m\n" __VA_OPT__(,) __VA_ARGS__)

#define BUILD_SYSTEM_COMPILE_MSG(command, ...) printf("\033[36m" "[Build system] Compiling: `" command "`\033[39m\n" __VA_OPT__(,) __VA_ARGS__)

#define BUILD_SYSTEM_EXEC_MSG(executable, ...) printf("\033[35m" "[Build system] Executing: `" executable "`\033[39m\n" __VA_OPT__(,) __VA_ARGS__)

typedef int32_t TargetId;

typedef enum
{
	STATIC_LIB,
	DYNAMIC_LIB,
	EXECUTABLE,
} BuildType;

// Intelligent self-rebuild logic
bool rebuild_self(const char* source, char** argv);

// Return the target index. This function can fail and exit the program.
const TargetId add_target(const char* name, const char* src, const char* include, const char* lib, const char* build, BuildType build_type);

const char* generate_compiler_command(const char* target_name);

const char* get_target_executable(const char* target_name);

