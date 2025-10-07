//
// Created by Rens on 8/09/25.
//

#ifndef DEFS_H
#define DEFS_H

#include <stdbool.h>
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

	PACKET_TUB_ARRIVE = 0,
	PACKET_PLANE_ARRIVE = 1,
	PACKET_TUB_MOVE = 2,

	/**
	 * A packet for updating a module's color.
	 *
	 * Format
	 * - char 0: TestPacketType
	 * - char 1: New R value
	 * - char 2: New G value
	 * - char 3: New B value
	 */
	PACKET_COLOR_UPDATE = 3,

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

typedef struct {

	uint8_t id;
	uint8_t departure_time;
	bool leaving;

} TestPlane;

typedef struct {

	uint8_t id;
	uint8_t plane_id;
	bool passed_security;
	bool needs_security;
	uint8_t destination_id;
	uint8_t destination_type;
	uint8_t priority;
	bool plane_arrived;
	bool plane_dropoff;
	bool safe;

} TestTub;

#endif // DEFS_H
