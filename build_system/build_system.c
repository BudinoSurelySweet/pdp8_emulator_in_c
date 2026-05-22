#include <stddef.h>
#include <stdio.h>
#include <iso646.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#include "build_system.h"

// TODO: Fare in modo che `targets` e `directories` siano vettori e non array, così che non devono avere una grandezza fissa

#define MAX_FIELD_LENGTH 128
#define MAX_TARGETS 16
#define MAX_DIRECTORIES 128
#define MAX_FILES_TO_COMPILE 128

typedef struct
{
	char name[MAX_FIELD_LENGTH + 1];
	char src[MAX_FIELD_LENGTH + 1];
	char include[MAX_FIELD_LENGTH + 1];
	char lib[MAX_FIELD_LENGTH + 1];
	char build[MAX_FIELD_LENGTH + 1];
	BuildType build_type;
} Target;

static Target targets[MAX_TARGETS];
static size_t targets_count;

bool rebuild_self(const char* source, char** argv)
{
    struct stat stat_src, stat_exec;
    char* executable = argv[0];

    if (stat(source, &stat_src) != 0) return false;

	if (stat(executable, &stat_exec) == 0 and stat_src.st_mtime <= stat_exec.st_mtime) return 0;

	printf("\033[33m[Build System] build.conf modified. Re-compiling build system...\033[0m\n");

	// Compile main.c (it include build.conf automaticly)
	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "gcc build_system/main.c build_system/build_system.c -o %s", executable);

	if (system(cmd) == 0)
	{
		printf("\033[32m[Build System] Re-compilation completed. Restarting build system...\033[0m\n");
		execv(executable, argv);
		return true;
	}
	else
	{
		fprintf(stderr, "\033[31m[Build System] Error during re-compilation of build system!\033[0m\n");
		exit(1);
	}

    return false;
}

static bool directory_exists(const char* relative_path)
{
	struct dirent* entry;
	DIR* directory;
	bool result = true;

	directory = opendir(relative_path);

	if (directory == NULL) result = false;

	closedir(directory);
	return result;
}

const TargetId add_target(const char* name, const char* src, const char* include, const char* lib, const char* build, BuildType build_type)
{
	BUILD_SYSTEM_ASSERT(directory_exists(src), "`%s` directory doesn't exist", src);
	BUILD_SYSTEM_ASSERT(targets_count < MAX_TARGETS, "there are too many targets (%zu/%d)", targets_count, MAX_TARGETS);
	BUILD_SYSTEM_ASSERT(strlen(name) <= MAX_FIELD_LENGTH, "`name` parameter in `add_target` function is too long");

	for (int i = 0; i < targets_count; i++)
	{
		BUILD_SYSTEM_ASSERT(strcmp(targets[i].name, name) != 0, "There are already a target named `%s`", name);
	}

	BUILD_SYSTEM_ASSERT(strlen(src) <= MAX_FIELD_LENGTH, "`src` parameter in `add_target` function is too long");
	BUILD_SYSTEM_ASSERT(strlen(include) <= MAX_FIELD_LENGTH, "`include` parameter in `add_target` function is too long");
	BUILD_SYSTEM_ASSERT(strlen(lib) <= MAX_FIELD_LENGTH, "`lib` parameter in `add_target` function in is too long");
	BUILD_SYSTEM_ASSERT(strlen(build) <= MAX_FIELD_LENGTH, "`build` parameter in `add_target` function in is too long");

	// Add target to array
	strcpy(targets[targets_count].name, name);
	strcpy(targets[targets_count].src, src);
	strcpy(targets[targets_count].include, include);
	strcpy(targets[targets_count].lib, lib);
	strcpy(targets[targets_count].build, build);
	targets[targets_count].build_type = build_type;

	targets_count++;

	return targets_count;
}

const char* generate_compiler_command(const char* target_name)
{
	Target* target = NULL;

	// TODO: Aggiungere una funzione che fa selezionare all'utente quale sia il target di default invece di selezionare il primo

	// Set the target to the first if not specified
	if (strcmp(target_name, NO_TARGET_MENTIONED) == 0)
	{
		target = &targets[0];
		BUILD_SYSTEM_MSG("Default target selected (`%s`)", target->name);
	}

	// Search the target name inside the array
	for (int i = 0; i < targets_count and not target; i++)
	{
		if (strcmp(targets[i].name, target_name) == 0)
		{
			target = &targets[i];
			BUILD_SYSTEM_MSG("Target selected (`%s`)", target->name);
		}
	}

	BUILD_SYSTEM_ASSERT(target, "`%s` isn't a target", target_name);

	char* directories[MAX_DIRECTORIES];
	size_t directories_count = 0;

	char* files_to_compile[MAX_FILES_TO_COMPILE];
	size_t files_count = 0;

	directories[0] = target->src;
	directories_count++;

	// Read all directories
	for (size_t dir_i = 0; dir_i < directories_count; dir_i++)
	{
		struct dirent* entry;
		DIR* dir = opendir(directories[dir_i]);

		BUILD_SYSTEM_ASSERT(dir, "`%s` directory doesn't exist", directories[dir_i]);

		while ((entry = readdir(dir)))
		{
			switch (entry->d_type) {
				case DT_DIR:
					if (strcmp(entry->d_name, ".") == 0 or strcmp(entry->d_name, "..") == 0) break;

					BUILD_SYSTEM_ASSERT(directories_count < MAX_DIRECTORIES, "Too many subdirectories (max %d)", MAX_DIRECTORIES);

					size_t dir_len = strlen(directories[dir_i]);
					bool needs_slash = (dir_len > 0 && directories[dir_i][dir_len - 1] != '/');
					size_t path_len = dir_len + (needs_slash ? 1 : 0) + strlen(entry->d_name) + 1;
					char* full_path = malloc(path_len);
					BUILD_SYSTEM_ASSERT(full_path, "Memory allocation failed");

					snprintf(full_path, path_len, "%s%s%s", directories[dir_i], needs_slash ? "/" : "", entry->d_name);

					directories[directories_count] = full_path;
					directories_count++;

					break;

				case DT_REG:
					BUILD_SYSTEM_ASSERT(files_count < MAX_FILES_TO_COMPILE, "Too many files (max %d)", MAX_FILES_TO_COMPILE);

					// FIX: Construct the full path and allocate memory
					size_t dir_len_f = strlen(directories[dir_i]);
					bool needs_slash_f = (dir_len_f > 0 && directories[dir_i][dir_len_f - 1] != '/');
					size_t path_len_f = dir_len_f + (needs_slash_f ? 1 : 0) + strlen(entry->d_name) + 1;
					
					char* file_path = malloc(path_len_f);
					BUILD_SYSTEM_ASSERT(file_path, "Memory allocation failed");
					snprintf(file_path, path_len_f, "%s%s%s", directories[dir_i], needs_slash_f ? "/" : "", entry->d_name);

					files_to_compile[files_count] = file_path;
					files_count++;

					break;

				case DT_UNKNOWN:
					// TODO: Implementare la ricerca di directory o file che non dipenda da `dirent`
					break;

				default:
					// TODO: Questo caso non dovrebbe mai esistere (in caso fare un assert)
					break;
			}
		}

		closedir(dir);
	}

	// Calculate total length for the files string
	size_t files_str_len = 0;
	for (size_t i = 0; i < files_count; i++)
	{
		files_str_len += strlen(files_to_compile[i]) + 1;
	}

	char* files_str = malloc(files_str_len + 1);
	BUILD_SYSTEM_ASSERT(files_str, "Memory allocation failed");
	files_str[0] = '\0';

	// Concatenate files
	for (size_t i = 0; i < files_count; i++)
	{
		strcat(files_str, files_to_compile[i]);
		if (i < files_count - 1) strcat(files_str, " ");
	}

	// Generate the final command
	size_t command_len = files_str_len + strlen(target->include) + strlen(target->lib) + strlen(target->name) + 64;
	char* command = malloc(command_len);
	BUILD_SYSTEM_ASSERT(command, "Memory allocation failed");

	snprintf(command, command_len, "gcc %s -I%s -L%s -o %s%s", files_str, target->include, target->lib, target->build, target->name);

	free(files_str);

	// Cleanup allocated paths
	for (size_t i = 1; i < directories_count; i++)
	{
		free(directories[i]);
	}

	for (size_t i = 0; i < files_count; i++)
	{
		free(files_to_compile[i]);
	}
	
	return command;
}


const char* get_target_executable(const char* target_name)
{
	Target* target = NULL;

	// Set the target to the first if not specified
	if (strcmp(target_name, NO_TARGET_MENTIONED) == 0)
		target = &targets[0];

	// Search the target name inside the array
	for (int i = 0; i < targets_count and not target; i++)
	{
		if (strcmp(targets[i].name, target_name) == 0) target = &targets[i];
	}

	BUILD_SYSTEM_ASSERT(target, "`%s` isn't a target", target_name);

	size_t command_len = strlen("./") + strlen(target->build) + strlen(target->name) + 1;
	char* command = malloc(command_len);

	snprintf(command, command_len, "./%s%s", target->build, target->name);

	return command;
}

