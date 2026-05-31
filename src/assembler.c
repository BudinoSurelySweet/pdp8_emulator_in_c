#include "pdp8.h"
#include <assembler.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define HASHMAP_IMPL
#include <hashmap.h>

#define TOKEN_CAPACITY 32

static uint16_t program_counter = 0;

typedef struct
{
    char* data;
    size_t size;
} Span;

typedef struct
{
	uint16_t i : 1;
	uint16_t opr : 3;
	uint16_t addr : 12;
	bool is_completed;
	bool started;
} Instruction;

typedef enum
{
	INSTRUCTION,
	LABEL,
	INSTRUCTION_OR_I,
	DECIMAL_NUMBER,
	HEXADECIMAL_NUMBER,
	START_OF_PROGRAM,
} NextToken;

typedef struct
{
	char data[TOKEN_CAPACITY + 1];
	size_t size;
} Token;

static Span load_file_content(const char* path)
{
    if (!path) return (Span){NULL, 0};
    FILE* file = fopen(path, "rb");

    if (!file)
    {
        printf("Failed to open file at path %s\n", path);
        return (Span){NULL, 0};
    }

    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* data = (char*)malloc(length + 1 /*null terminator*/);

    if (!data) return (Span){NULL, 0};

    fread(data, 1, length, file);
    fclose(file);

    data[length] = '\0';

    return (Span){data, length};
}

static inline void free_span(Span* span)
{
    if (!span || !span->data) return;
    free(span->data);
    span->data = NULL;
    span->size = 0;
}

// DEPRECATED: Remove this function 'cause it's unused
static bool str_equal(const char* s1, const char* s2)
{
	return strcmp(s1, s2) == 0;
}

// TEST:
// Rimuove i caratteri da 'start' fino a 'end' (escluso)
void remove_substring(char *str, int start, int end) {
    if (str == NULL) return;

    int len = strlen(str);

    // Controlli di sicurezza per evitare buffer overflow
    if (start < 0) start = 0;
    if (end > len) end = len;
    if (start >= end) return; // Niente da rimuovere

    // Calcoliamo quanti byte dobbiamo spostare.
    // Il +1 è cruciale: serve a copiare anche il carattere terminatore '\0'
    int bytes_to_move = len - end + 1;

    // Sposta la memoria
    // Destinazione: l'indirizzo da cui vogliamo iniziare a sovrascrivere (str + start)
    // Origine: l'indirizzo del primo carattere da mantenere (str + end)
    memmove(str + start, str + end, bytes_to_move);
}

static void fill_instruction(Instruction* instruction_buffer, uint16_t instruction)
{
	instruction_buffer->i = instruction >> 15;
	instruction_buffer->opr = instruction >> 12;
	instruction_buffer->addr = instruction & 0x0fff;

	instruction_buffer->is_completed = true;
}

static void create_instruction(hashmap symbol_table, const char* token_buffer, word_t* ram, int32_t* line_num)
{
	static NextToken next_token = INSTRUCTION;
	static Instruction instruction_buffer = {0};

	size_t token_len = strlen(token_buffer);

	bool is_indirect_addressing = next_token == INSTRUCTION_OR_I && token_len == 1 && token_buffer[0] == 'I';
	bool is_instruction = (next_token == INSTRUCTION || next_token == INSTRUCTION_OR_I) && !is_indirect_addressing && token_len == 3;
	bool is_label = next_token == LABEL;
	bool is_dec_number = next_token == DECIMAL_NUMBER;
	bool is_hex_number = next_token == HEXADECIMAL_NUMBER;
	bool is_start_of_program = next_token == START_OF_PROGRAM;

	if (instruction_buffer.started && is_instruction)
	{
		ram[*line_num] = instruction_buffer.addr & 0x0FFF;
		ram[*line_num] |= (word_t) instruction_buffer.opr << 12;
		ram[*line_num] |= (word_t) instruction_buffer.i << 15; 

		memset(&instruction_buffer, 0, sizeof(Instruction));

		(*line_num)++;
	}

	if (is_indirect_addressing)
	{
		next_token = INSTRUCTION;
		instruction_buffer.i = 1;
		instruction_buffer.is_completed = true;
	}
	else if (is_instruction)
	{
		char first = token_buffer[0];
		char second = token_buffer[1];
		char third = token_buffer[2];

		switch (first)
		{
			case 'A':
				if (second == 'D' && third == 'D') instruction_buffer.opr = ADD;
				else if (second == 'N' && third == 'D') instruction_buffer.opr = AND;
				else ERROR("Token isn't an instruction: %s", token_buffer);

				instruction_buffer.started = true;
				next_token = LABEL;

				break;

			case 'B':
				if (second == 'S' && third == 'A') instruction_buffer.opr = BSA;
				else if (second == 'U' && third == 'N') instruction_buffer.opr = BUN;
				else ERROR("Token isn't an instruction: %s", token_buffer);

				instruction_buffer.started = true;
				next_token = LABEL;

				break;

			case 'C':
				switch (second)
				{
					case 'I':
						if (third == 'L') fill_instruction(&instruction_buffer, CIL);
						else if (third == 'R') fill_instruction(&instruction_buffer, CIR);
						else ERROR("Token isn't an instruction: %s", token_buffer);

						next_token = INSTRUCTION;

						break;

					case 'L':
						if (third == 'A') fill_instruction(&instruction_buffer, CLA);
						else if (third == 'E') fill_instruction(&instruction_buffer, CLE);
						else ERROR("Token isn't an instruction: %s", token_buffer);

						next_token = INSTRUCTION;

						break;

					case 'M':
						if (third == 'A') fill_instruction(&instruction_buffer, CMA);
						else if (third == 'E') fill_instruction(&instruction_buffer, CME);
						else ERROR("Token isn't an instruction: %s", token_buffer);

						next_token = INSTRUCTION;

						break;

					default: ERROR("Token isn't an instruction: %s", token_buffer);
				}

				break;

			case 'D':
				if (second == 'E' && third == 'C') next_token = DECIMAL_NUMBER;
				else ERROR("Token isn't an instruction: %s", token_buffer);

				break;

			case 'E':
				if (second == 'N' && third == 'D')
				{
					UNIMPLEMENTED;
				}
				else ERROR("Token isn't an instruction: %s", token_buffer);

				break;

			case 'H':
				if (second == 'E' && third == 'X')
				{
					next_token = HEXADECIMAL_NUMBER;
				}
				else if (second == 'L' && third == 'T')
				{
					fill_instruction(&instruction_buffer, HLT);

					next_token = INSTRUCTION;
				}
				else ERROR("Token isn't an instruction: %s", token_buffer);

				break;

			case 'I':
				switch (second)
				{
					case 'N':
						if (third == 'C') fill_instruction(&instruction_buffer, INC);
						else if (third == 'P') fill_instruction(&instruction_buffer, INP);
						else ERROR("Token isn't an instruction: %s", token_buffer);

						next_token = INSTRUCTION;

						break;

					case 'S':
						if (third == 'Z') instruction_buffer.opr = ISZ;
						else ERROR("Token isn't an instruction: %s", token_buffer);

						instruction_buffer.started = true;
						next_token = LABEL;

						break;

					default: ERROR("Token isn't an instruction: %s", token_buffer);
				}

				break;
			
			case 'L':
				if (second == 'D' && third == 'A') instruction_buffer.opr = LDA;
				else ERROR("Token isn't an instruction: %s", token_buffer);

				instruction_buffer.started = true;
				next_token = LABEL;

				break;

			case 'O':
				if (second == 'R' && third == 'G')
				{
					 next_token = START_OF_PROGRAM;
				}
				else if (second == 'U' && third == 'T')
				{
					fill_instruction(&instruction_buffer, OUT);

					next_token = INSTRUCTION;
				}
				else ERROR("Token isn't an instruction: %s", token_buffer);

				break;

			case 'S':
				switch (second)
				{
					case 'N':
						if (third == 'A') fill_instruction(&instruction_buffer, SNA);
						else ERROR("Token isn't an instruction: %s", token_buffer);

						next_token = INSTRUCTION;

						break;

					case 'P':
						if (third == 'A') fill_instruction(&instruction_buffer, SPA);
						else ERROR("Token isn't an instruction: %s", token_buffer);

						next_token = INSTRUCTION;

						break;

					case 'T':
						if (third == 'A') instruction_buffer.opr = STA;
						else ERROR("Token isn't an instruction: %s", token_buffer);

						instruction_buffer.started = true;
						next_token = LABEL;

						break;

					case 'Z':
						if (third == 'A') fill_instruction(&instruction_buffer, SZA);
						else if (third == 'E') fill_instruction(&instruction_buffer, SZE);
						else ERROR("Token isn't an instruction: %s", token_buffer);

						next_token = INSTRUCTION;

						break;

					default: ERROR("Token isn't an instruction: %s", token_buffer);
				}

				break;

			default: ERROR("Token isn't an instruction: %s", token_buffer);
		}
	}
	else if (is_label)
	{
		next_token = INSTRUCTION_OR_I;

		// TEST: 
		uint32_t value = 0;
		hashmap_get_val(symbol_table, token_buffer, &value);

		// printf("label: %s\nvalue: %d\n", token_buffer, value);
		
		instruction_buffer.addr = value + program_counter - 1;
	}
	else if (is_dec_number)
	{
		next_token = INSTRUCTION;

		fill_instruction(&instruction_buffer, (uint16_t) atoi(token_buffer));
	}
	else if (is_hex_number)
	{
		next_token = INSTRUCTION;

		fill_instruction(&instruction_buffer, (uint16_t) strtol(token_buffer, NULL, 16));
	}
	else if (is_start_of_program)
	{
		next_token = INSTRUCTION;

		program_counter = (uint16_t) strtol(token_buffer, NULL, 16);
		*line_num = program_counter;
	}

	if (instruction_buffer.is_completed)
	{
		ram[*line_num] = instruction_buffer.addr & 0x0FFF;
		ram[*line_num] |= (word_t) instruction_buffer.opr << 12;
		ram[*line_num] |= (word_t) instruction_buffer.i << 15; 

		memset(&instruction_buffer, 0, sizeof(Instruction));

		(*line_num)++;
	}
}

uint16_t assemble_and_load(PDP8* cpu, const char* path)
{
	hashmap symbol_table = {0};
	Span source_code = load_file_content(path);
	uint16_t possible_label_start_pos = 0;
	int32_t line_num = 0;
	Token token_buffer = {0};
	bool is_comment = false;

	// Fetch labels
	for (uint16_t i = 0; i < source_code.size; i++)
	{
		char c = source_code.data[i];

		if (c == '\t') continue;

		if (source_code.data[i - 1] == '\n' && c == '\n')
			ERROR("You can't have a blank line. Blank line found at %d", line_num + 1);

		switch (c)
		{
			case '/':
				is_comment = true;

				break;

			case '\n':
				possible_label_start_pos = i + 1;
				line_num++;
				is_comment = false;

				break;

			case ',':
				if (is_comment) continue;

				size_t size = i - possible_label_start_pos;
				char label[size + 1];

				// Set label to 0
				memset(label, 0, size + 1);

				// The point where the function `strncpy` start to copy
				const char* start = source_code.data + possible_label_start_pos;

				// Copy sub-string
				strncpy(label, start, size);

				// TODO: Add rules to labels (no space, no number at the start, no symbol except underscore)

				hashmap_set_val(symbol_table, label, line_num);

				// Remove the label to make easy for the second loop to fetch instructions
				// remove_substring(source_code.data, possible_label_start_pos, possible_label_start_pos + size + 2);
				memset(source_code.data + possible_label_start_pos, ' ', size + 1);

				break;
		}
	}

	line_num = 0;
	is_comment = false;

	// FIXME: After instruction is completed a new line is expected, but now it isn't.

	// Translate to binary
	for (uint16_t i = 0; i < source_code.size; i++)
	{
		char c = source_code.data[i];
		bool is_separator = c == ' ' || c == '\n' || c == '\t';

		if (c == '/') is_comment = true;
		else if (c == '\n') is_comment = false;

		if (c == '\t' || is_comment) continue;

		if (!is_separator)
		{
			token_buffer.data[token_buffer.size] = c;
			token_buffer.size++;
		}
		else if (strlen(token_buffer.data) != 0)
		{
			create_instruction(symbol_table, token_buffer.data, cpu->ram, &line_num);
			memset(&token_buffer, 0, sizeof(Token));
		}
	}

	hashmap_delete(symbol_table);
	free_span(&source_code);

	return program_counter;
}

