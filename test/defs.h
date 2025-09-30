//
// Created by Rens on 8/09/25.
//

#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>

/**
 * Max amount of processes that may exist alongside the Pi.
 */
#define MAX_PROCESSES 50

/**
 * Represents a packet that can be sent between Picos and the Pi.
 */
typedef struct {
	/**
	 * The id of the sender.
	 */
	uint8_t source;

	/**
	 * The size of the data.
	 */
	uint8_t size;

	/**
	 * The data.
	 */
	char data[256];
} Packet;

#endif // DEFS_H
