//
// Created by Rens on 4/09/25.
//

#include "defs.h"
#include "queue.h"

#include <assert.h>
#include <fcntl.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * The amount of processes created.
 */
static uint8_t processes = 0;

/**
 * Creates a new process which will execute the specified executable file.
 * @param process_name The name of the process (by convention).
 * @param file The file to execute.
 * @param arg The argument to pass to the executable.
 */
void create_process(const char* process_name, const char* file, const char* arg) {
	const pid_t pid = fork();

	if (pid == -1) {
		perror("Failed to fork");
		exit(EXIT_FAILURE);
	}

	const bool child = pid == 0;
	if (child) {
		printf("[Tests] Starting process %s at %d\n", arg, getpid());

		const int success = execl(file, process_name, arg, (char*)NULL);

		if (success < 0) {
			perror("Failed to execl file");
			exit(EXIT_FAILURE);
		}
	}
}

/**
 * Creates a pico instance.
 * @param id The id of the pico, as described in map.txt.
 * @param process_name The name of the process (by convention).
 * @param executable The name of the executable to use to create this process.
 * Usually <code>pico_default</code>, <code>pico_security</code> or similar.
 */
void run_instance(const uint8_t id, const char* process_name, const char* executable) {
	char name[24];
	char arg[12];
	char target_file[40];

	snprintf(name, sizeof(name), "%s %d", process_name, id);
	snprintf(arg, sizeof(arg), "%d", id);
	snprintf(target_file, sizeof(target_file), "../target/%s", executable);

	printf("[Tests] Starting %s %d\n", process_name, id);
	create_process(name, target_file, arg);

	processes++;
}

/**
 * Parses the map.txt file and creates the according queues and processes per line.
 */
void parse_map(void) {
	FILE* file = fopen("../map.txt", "r");

	if (file == NULL) {
		perror("Failed to read map.txt");
		exit(EXIT_FAILURE);
	}

	regex_t regex;
	regmatch_t matches[4]; // full match + 2 groups
	const int regex_result = regcomp(&regex, "(pi|pico[0-9]+)[[:space:]]+(.*?/)?([^/]*).c", REG_EXTENDED);

	if (regex_result < 0) {
		perror("Failed to compile regex");
		fclose(file);
		exit(EXIT_FAILURE);
	}

	char line[100];
	while (true) {
		const char* read = fgets(line, sizeof(line), file);
		if (read == NULL) {
			break;
		}

		if (regexec(&regex, line, 4, matches, 0) != 0) {
			perror("Line does not match map.txt format of (pi|pico[0-9]+)[[:space:]]+(.*?/)?([^/]*).c");
			regfree(&regex);
			fclose(file);
			exit(EXIT_FAILURE);
		}

		// get the parts of the line: pico(id)/pi + file
		const int typeLen = matches[1].rm_eo - matches[1].rm_so;
		char type[typeLen + 1];
		strncpy(type, line + matches[1].rm_so, typeLen);
		type[typeLen] = '\0';

		const int executableLen = matches[3].rm_eo - matches[3].rm_so;
		char executable[executableLen + 1];
		strncpy(executable, line + matches[3].rm_so, executableLen);
		executable[executableLen] = '\0';

		if (type[2] == 'c') {
			// this is a pico
			uint8_t id = type[4] - '0';
			if (typeLen > 5) {
				id = id * 10 + (type[5] - '0');
			}

			assert(id > 0);

			run_instance(id, "Pico", executable);
		} else {
			run_instance(0, "Pi", executable);
		}
	}

	regfree(&regex);
	fclose(file);
}

int main(int argc, char* argv[]) {

	// first create internal system processes
	create_process("Display", "../target/display", "");
	create_process("Manager", "../target/manager", "");

	// create all the processes that represent picos
	parse_map();

	for (int i = 0; i < processes + 1; ++i) {
		wait(NULL);
	}

	// run tests on all of them

	// collect results

	return EXIT_SUCCESS;
}
