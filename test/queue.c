//
// Created by Rens on 8/09/25.
//

#include "queue.h"
#include "defs.h"

#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>

mqd_t open_queue(const char* queue_name, const int flags) {
	printf("[Tests] Opening queue with name %s\n", queue_name);

	const struct mq_attr attributes = {
		.mq_flags = 0, // ignored with mq_open
		.mq_maxmsg = MQ_MAX_MESSAGES,
		.mq_msgsize = sizeof(Packet),
		.mq_curmsgs = 0 // ignored with mq_open
	};

	const mqd_t queue = mq_open(queue_name, flags,
		S_IRUSR | S_IWUSR,
		&attributes);

	if (queue == -1) {
		perror("Creating test queue failed");
		exit(EXIT_FAILURE);
	}

	return queue;
}

ssize_t receive_queue(const mqd_t id, Packet* packet) {
	return mq_receive(id, (char*) packet, sizeof(Packet), 0);
}

int send_queue(const mqd_t id, Packet* packet) {
	return mq_send(id, (char*) packet, sizeof(Packet), 0);
}

void close_queue(const mqd_t id, const char* queue_name) {
	printf("[Tests] Closing %s queue\n", queue_name);

	if (mq_close(id) == -1) {
		perror("Closing queue failed");
		// do not exit as we can still try to close the other queues
	}

	if (mq_unlink(queue_name) == -1) {
		perror("Unlinking queue failed");
		// do not exit as we can still try to close the other queues
	}
}
