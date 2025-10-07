//
// Created by Rens on 6/09/25.
//

#define Q_TEST
#ifdef Q_TEST

#include "../src/quercus_lib_pico.h"
#include "queue.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static const char* name;

/**
 * @brief Generic setup function for Picos.
 * @param argc The argument count.
 * @param argv The passed arguments.
 * @param loop_fn_ptr A pointer to the function to execute as loop.
 * @return The exit code for the main function.
 */
int main_loop(const int argc, char* argv[], void (*loop_fn_ptr)(void)) {
	name = argv[0];

	printf("[%s] Enabled\n", name);

#ifndef Q_DEBUG
	if (argc != 2) {
		perror("Test mode cannot be instantiated with no mq designation");
		return EXIT_FAILURE;
	}

	const char* cid = argv[1];
	char* end;

	const long id = strtol(cid, &end, 10);
	if (cid == end) {
		printf("[%s] Empty id passed\n", argv[1]);
		return EXIT_FAILURE;
	}
	if (*end != '\0') {
		printf("[%s] Invalid id char '%c'\n", argv[1], *end);
		return EXIT_FAILURE;
	}
	if (id < INT_MIN || id > INT_MAX) {
		printf("[%s] Id %ld out of range\n", argv[1], id);
		return EXIT_FAILURE;
	}
#else
	const long id = 0;
	const char cid[] = "0";
#endif

	initialize_q((int)id); // Testing: test-only function

	assert(get_own_id() == id);

	loop_fn_ptr();

	return EXIT_SUCCESS;
}

#endif
