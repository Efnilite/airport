//
// Created by efy on 8/10/25.
//

#include "../queue.h"
#include "../test.h"

#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>

/**
 * The id of the manager queue.
 */
static mqd_t display_queue;

/**
 * The queue ids of all the other target message queues.
 */
static mqd_t queue_ids[MAX_PROCESSES];

static void open_manager_queue(const uint8_t module) {
	char name[50];
	snprintf(name, sizeof(name), "/QA to %d", module);
	printf("[Manager] Opening queue to %d\n", module);
	queue_ids[module] = open_queue(name, O_WRONLY);
}

int main(int argc, char* argv[]) {

	display_queue = open_queue("/QA to Manager", O_RDONLY | O_CREAT | O_EXCL);

	sleep(5);

	{
		open_manager_queue(TEST_TUB_MODULE);
		open_manager_queue(TEST_PLANE_MODULE);
	}

	sleep(5);

	{
		Packet packet;
		packet.type = TYPE_TEST;
		packet.source = 0;
		packet.size = DATA_RFID_LENGTH + 1;

		packet.data[0] = PACKET_TUB_ARRIVE;
		packet.data[DATA_TUB_OR_PLANE + 1] = 0;
		packet.data[DATA_TUB_ID + 1] = (char)TEST_TUB.id;
		packet.data[DATA_PASSED_SECURITY + 1] = (char)TEST_TUB.passed_security;
		packet.data[DATA_NEEDS_SECURITY + 1] = (char)TEST_TUB.needs_security;
		packet.data[DATA_PLANE_OR_DROPOFF + 1] = (char)TEST_TUB.plane_dropoff;
		packet.data[DATA_PLANE_ARRIVED + 1] = (char)TEST_TUB.plane_arrived;
		packet.data[DATA_PAYLOAD + 1] = (char)TEST_TUB.safe;
		packet.data[DATA_PLANE_ID + 1] = (char)TEST_TUB.plane_id;
		packet.data[DATA_DESTINATION + 1] = (char)TEST_TUB.destination_id;

		const int queue = send_queue(queue_ids[TEST_TUB_MODULE], &packet);
		if (queue < 0) {
			fprintf(stderr, "[Manager] Failed to send tub init\n");
		}
	}

	sleep(2);

	{
		Packet packet;
		packet.type = TYPE_TEST;
		packet.source = 0;
		packet.size = DATA_RFID_LENGTH + 1;

		packet.data[0] = PACKET_PLANE_ARRIVE;
		packet.data[DATA_TUB_OR_PLANE + 1] = 1;
		packet.data[DATA_PLANE_ID + 1] = (char)TEST_PLANE.id;
		packet.data[DATA_DEPARTURE_TIME + 1] = (char)TEST_PLANE.departure_time;
		packet.data[DATA_PLANE_DIRECTION + 1] = (char)TEST_PLANE.leaving;

		const int queue = send_queue(queue_ids[TEST_PLANE_MODULE], &packet);
		if (queue < 0) {
			fprintf(stderr, "[Manager] Failed to send plane init\n");
		}
	}

}
