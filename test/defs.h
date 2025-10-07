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

typedef enum {
	/**
	 * Represents any message intended to be read by the Pico/Pi implementation.
	 */
	TYPE_SYSTEM,
	/**
	 * Represents any message sent to a Pico/Pi by the testing infrastructure.
	 */
	TYPE_TEST,

} PacketType;

typedef enum {

	/**
	 * A packet for updating a module's color.
	 *
	 * ### Format
	 * - char 0: TestPacketType
	 * - char 1: New R value
	 * - char 2: New G value
	 * - char 3: New B value
	 */
	PACKET_COLOR_UPDATE,

} TestPacketType;

/**
 * Represents a packet that can be sent between Picos and the Pi.
 */
typedef struct {
	/**
	 * The id of the sender.
	 */
	uint8_t source;

	/**
	 * The type of the packet.
	 */
	PacketType type;

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
