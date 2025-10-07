//
// Created by Rens on 4/09/25.
//

#include "../src/quercus_lib_pico.h"
#include "defs.h"
#include "queue.h"

#include <assert.h>
#include <linux/kernel.h>
#include <math.h>
#include <mqueue.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <time.h>

#define MS_TO_NS 1000000

// Generic functions
void sleep(const int ms) {
	const struct timespec ts = {.tv_nsec = ms * MS_TO_NS};

	nanosleep(&ts, NULL); // use nanosleep from time.h to avoid importing unistd sleep
}

/**
 * The queue id of the display communication.
 */
static mqd_t display_queue;

/**
 * The queue ids of all the other target message queues.
 */
static mqd_t queue_ids[MAX_PROCESSES];

/**
 * The module id.
 */
static int q_id = -1;

void initialize_q(const int qid) {
	assert(q_id == -1);
	assert(qid >= 0);

	// initialize array
	for (int i = 0; i < MAX_PROCESSES; ++i) {
		queue_ids[i] = -1;
	}

	q_id = qid;

	char queue_name[50];
	snprintf(queue_name, sizeof(queue_name), "/QA to %d", qid);
	queue_ids[qid] = open_queue(queue_name, O_RDONLY | O_CREAT | O_EXCL | O_NONBLOCK);
	display_queue = open_queue("/QA to Display", O_WRONLY);
}

// Module functions
int get_own_id() { return q_id; }

static char network_map[4096];

/**
 * Reads the network map into a buffer.
 * @param buffer The buffer variable.
 * @param buffer_size The size of the buffer.
 * @return The amount of read bytes.
 */
// shamelessly stolen from
// https://gitlab.tue.nl/2irr70-capstone-quercus-airport/quercus_pi_libcoap/-/blob/main/src/coap/pico_monitor.c?ref_type=heads#L111
size_t read_network_map(char* buffer, const size_t buffer_size) {
	FILE* file = fopen("../network_map", "r");
	if (file == NULL) {
		perror("Failed to open network map file");
		return -1;
	}

	const size_t bytes_read = fread(buffer, sizeof(char), buffer_size - 1, file); // Reserve space for null terminator
	if (bytes_read == 0 && !feof(file)) { // Check if read failed (not end-of-file)
		perror("Failed to read network map file");
		fclose(file);
		return -1;
	}

	// Null terminate the buffer
	buffer[bytes_read] = '\0';

	fclose(file);
	return bytes_read;
}

char* get_network_map() {
	read_network_map(network_map, sizeof(network_map));

	return network_map;
}

int get_uptime() {
	struct sysinfo s;

	if (sysinfo(&s) < 0) {
		perror("Failed to get uptime");
		exit(EXIT_FAILURE);
	}

	return (int)s.uptime;
}

/**
 * A bitmask with all supported events.
 */
uint8_t events = 0;

// Event functions
void subscribe_to_event(const int event_type) { events |= 1 << event_type; }

// Tests: events are not subscribed to,
// push events with push_event(EventType event_type)
void unsubscribe_from_event(const int event_type) { events &= ~(1 << event_type); }

Packet recent_read;

EventType next_event() {
	Packet packet;

	const size_t size = receive_queue(queue_ids[get_own_id()], &packet);

	if (size == (size_t)-1) {
		return EVENT_NONE;
	}

	recent_read = packet;

	return EVENT_MESSAGE_RECEIVED;
}

// Laser functions

bool left_laser_on = false;
bool right_laser_on = false;

bool laser_left_detected = false;
bool laser_right_detected = false;

int laser_right_detect() { return laser_right_detected; }

int laser_left_detect() { return laser_left_detected; }

int laser_right_get() { return right_laser_on; }

int laser_right_set(const int on) {
	right_laser_on = on;
	return on;
}

int laser_left_get() { return left_laser_on; }

int laser_left_set(const int on) {
	left_laser_on = on;
	return on;
}

// Servo functions

float servo_angle = 0;

int servo_angle_set(const float angle) {
	servo_angle = angle;
	return (int)angle;
}

float servo_angle_get() { return servo_angle; }

// LED functions

extern int led_set_color(const int color) {
	Packet packet;
	packet.type = TYPE_TEST;
	packet.source = get_own_id();
	packet.size = 4;

	packet.data[0] = PACKET_COLOR_UPDATE;
	packet.data[1] = (char) (color >> 16 & 0xff);
	packet.data[2] = (char) (color >> 8 & 0xff);
	packet.data[3] = (char) (color & 0xff);

	send_queue(display_queue, &packet);

	return 0;
}

extern int led_set_rgb(const int r, const int g, const int b) {
	Packet packet;
	packet.type = TYPE_TEST;
	packet.source = get_own_id();
	packet.size = 4;

	packet.data[0] = PACKET_COLOR_UPDATE;
	packet.data[1] = (char) r;
	packet.data[2] = (char) g;
	packet.data[3] = (char) b;

	send_queue(display_queue, &packet);

	return 0;
}

extern int led_get_color() {
	fprintf(stderr, "Accessing LED color in tests is not allowed");
	exit(EXIT_FAILURE);
}

// Current-voltage sensor functions

// Tests: getting sensor voltage is unsupported
float CV_sensor_get_voltage() {
	perror("Sensor voltage call unimplemented");
	exit(EXIT_FAILURE);
}

float CV_sensor_get_power() {
	perror("Sensor power call unimplemented");
	exit(EXIT_FAILURE);
}

float CV_sensor_get_current() {
	perror("Sensor current call unimplemented");
	exit(EXIT_FAILURE);
}

float small_belt_speed = 0;

// Small belt functions
int belt_small_set_speed(const float speed) {
	small_belt_speed = speed;
	return (int)speed;
}

float belt_small_get_speed() { return small_belt_speed; }

// Tests: methods not used
int64_t belt_small_get_encoder_count() {
	perror("Small belt encoder count unimplemented");
	exit(EXIT_FAILURE);
}

double belt_small_get_encoder_freq() {
	perror("Small belt freq unimplemented");
	exit(EXIT_FAILURE);
}

float big_belt_speed = 0;

// Big belt functions

int belt_big_set_speed(const float speed) {
	big_belt_speed = speed;
	return (int)speed;
}

float belt_big_get_speed() { return big_belt_speed; }

// Tests: methods not used
int64_t belt_big_get_encoder_count() {
	perror("Big belt encoder count unimplemented");
	exit(EXIT_FAILURE);
}

double belt_big_get_encoder_freq() {
	perror("Big belt freq unimplemented");
	exit(EXIT_FAILURE);
}

// DIR_RFID functions
int RFID_check_tag() { return 0; }

int RFID_write_data_block(int data_ptr, int offset) { return 0; }

int RFID_read_data_block(int data_ptr, int offset) { return 0; }

int RFID_get_uid(int uid_pointer) { return 0; }

int next_message_address(uint8_t** address_ptr) {
	if (recent_read.source == (uint8_t)-1) {
		*address_ptr = NULL;
		return 0;
	}

	const Packet packet = recent_read;

	uint8_t* msg = malloc(packet.size);
	if (!msg) {
		perror("Failed to allocate message buffer");
		*address_ptr = NULL;
		return -1;
	}

	memcpy(msg, packet.data, packet.size);
	*address_ptr = msg;

	printf("[%d] Received packet from %d -> size %d ", get_own_id(), packet.source, packet.size);
	for (int i = 0; i < packet.size; ++i) {
		printf("%d ", (int)packet.data[i]);
	}
	printf("\n");

	recent_read = (Packet){.source = -1, .type = -1, .size = -1, .data = ""};

	return packet.size;
}

/**
 * Send a packet to the IP address with the last octet given by last_dest_octet.
 * The packet is sent with the data in the buffer at app_data, with the size of data_size.
 */
int send_packet(const int last_dest_octet, const char* app_data, const int data_size) {
	if (data_size > sizeof(Packet) - sizeof(uint8_t)) {
		perror("Packet size is bigger than allowed");
		return -1;
	}

	if (queue_ids[last_dest_octet] == -1) {
		char name[50];
		snprintf(name, sizeof(name), "/QA to %d", last_dest_octet);
		printf("[%d] Opening queue to %d during packet send\n", get_own_id(), last_dest_octet);
		queue_ids[last_dest_octet] = open_queue(name, O_WRONLY | O_NONBLOCK);
	}

	Packet packet;
	packet.source = get_own_id();
	packet.size = data_size;
	packet.type = TYPE_SYSTEM;
	for (int i = 0; i < data_size; ++i) {
		packet.data[i] = app_data[i];
	}

	const mqd_t id = queue_ids[last_dest_octet];

	assert(id >= 0);

	const int success = mq_send(id, (char*)&packet, sizeof(Packet), 0);

	if (success == -1) {
		perror("Failed to send packet");
		return -1;
	}

	printf("[%d] Sent packet to %d (size %d): ", get_own_id(), last_dest_octet, data_size);
	for (int i = 0; i < data_size; ++i) {
		printf("%d ", (int)app_data[i]);
	}
	printf("\n");

	return EXIT_SUCCESS;
}
