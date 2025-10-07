//
// Created by efy on 7/10/25.
//

#include "display.h"
#include "defs.h"
#include "queue.h"

#include <mqueue.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * The id of the display queue.
 */
static mqd_t display_queue;

/**
 * Returns the colors of all modules.
 */
static unsigned long colors[MAX_PROCESSES];

/**
 * @param r The red value
 * @param g The green value
 * @param b The blue value
 * @return The hex representation of this rgb value
 */
static unsigned long rgb_to_hex(const unsigned int r, const unsigned int g, const unsigned int b) {
	return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

int main(int argc, char* argv[]) {

	display_queue = open_queue("/QA to Display", O_RDONLY | O_CREAT | O_EXCL);

	while (true) {
		printf("[Display] Waiting for a display message\n");

		Packet packet;
		const size_t size = receive_queue(display_queue, &packet);

		if (size == (size_t)-1 || packet.type != TYPE_TEST) {
			continue;
		}

		const TestPacketType type = (unsigned char) packet.data[0];

		switch (type) {
		case PACKET_COLOR_UPDATE:
			const unsigned int r = (unsigned char)packet.data[1];
			const unsigned int g = (unsigned char)packet.data[2];
			const unsigned int b = (unsigned char) packet.data[3];

			colors[packet.source] = rgb_to_hex(r, g, b);
			printf("[Display] Updating color %d to %d,%d,%d\n", packet.source, r, g, b);
			break;
		default:
			fprintf(stderr, "Invalid packet type %d", type);
			exit(EXIT_FAILURE);
		}
	}
}
