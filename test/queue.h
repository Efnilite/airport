//
// Created by efy on 10/09/25.
//

#ifndef QUEUE_H
#define QUEUE_H

#include "defs.h"

#include <mqueue.h>

// The most amount of messages allowed in a queue.
#define MQ_MAX_MESSAGES 50

/**
 * @brief Opens a queue.
 * @param queue_name The name of the queue
 * @param flags The flags to use when opening this queue.
 * @return The queue id.
 */
mqd_t open_queue(const char* queue_name, int flags);

/**
 * Receives a value from a queue.
 * @param id The id of the queue.
 * @param packet A pointer to the packet that will be updated.
 * @return The size of the read values.
 */
size_t receive_queue(mqd_t id, Packet* packet);

/**
 * Sends a value to a queue.
 * @param id The id of the queue.
 * @param packet A pointer to the packet that will be sent.
 * @return -1 if send failed, 0 if ok.
 */
int send_queue(mqd_t id, Packet* packet);

/**
 * @brief Closes a single queue.
 * @param id The id of the queue
 * @param queue_name The name of the queue
 */
void close_queue(mqd_t id, const char* queue_name);

#endif //QUEUE_H
