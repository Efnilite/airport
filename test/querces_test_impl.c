//
// Created by Rens on 4/09/25.
//

#include "../src/defs.h"
#include "../src/quercus_lib_pico.h"
#include "defs.h"
#include "queue.h"
#include "test.h"

#include <assert.h>
#include <math.h>
#include <mqueue.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

char recent_rfid[DATA_RFID_LENGTH];

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
	display_queue = open_queue("/QA to Display", O_WRONLY | O_CREAT);

	recent_rfid[0] = (char) 1;
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
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
		return (int) (ts.tv_sec * 1000ull + ts.tv_nsec / 1000000ul);
	}
	return 0;
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

Packet recent_read = (Packet){.source = -1, .type = -1, .size = -1, .data = ""};
Packet recent_test_read = (Packet){.source = -1, .type = -1, .size = -1, .data = ""};

bool laser_left_detected = false;
bool laser_right_detected = false;

EventType next_event() {
	Packet packet;

	const ssize_t size = receive_queue(queue_ids[get_own_id()], &packet);

	if (size == (ssize_t)-1) {
		return EVENT_NONE;
	}

	if (packet.type == TYPE_SYSTEM) {
		recent_read = packet;

		return EVENT_MESSAGE_RECEIVED;
	}

	recent_test_read = packet;

	return EVENT_NONE;
}

// Laser functions

int laser_right_detect() {
	if (laser_right_detected) {
		laser_right_detected = false;
		fprintf(stderr, "[%d] Updated right laser to false\n", get_own_id());
		return true;
	}
	return false;
}

int laser_left_detect() {
	if (laser_left_detected) {
		laser_left_detected = false;
		fprintf(stderr, "[%d] Updated left laser to false\n", get_own_id());
		return true;
	}
	return false;
}

int laser_right_get() {
	fprintf(stderr, "Right laser get unimplemented\n");
	exit(EXIT_FAILURE);
}

int laser_right_set(const int on) {
	return on;
}

int laser_left_get() {
	fprintf(stderr, "Left laser get unimplemented\n");
	exit(EXIT_FAILURE);
}

int laser_left_set(const int on) {
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

	packet.data[0] = DISPLAY_COLOR_UPDATE;
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

	packet.data[0] = DISPLAY_COLOR_UPDATE;
	packet.data[1] = (char) r;
	packet.data[2] = (char) g;
	packet.data[3] = (char) b;

	send_queue(display_queue, &packet);

	return 0;
}

extern int led_get_color() {
	fprintf(stderr, "Led get color unimplemented\n");
	exit(EXIT_FAILURE);
}

// Current-voltage sensor functions

// Tests: getting sensor voltage is unsupported
float CV_sensor_get_voltage() {
	fprintf(stderr, "Sensor voltage call unimplemented\n");
	exit(EXIT_FAILURE);
}

float CV_sensor_get_power() {
	fprintf(stderr, "Sensor power call unimplemented\n");
	exit(EXIT_FAILURE);
}

float CV_sensor_get_current() {
	fprintf(stderr, "Sensor current call unimplemented\n");
	exit(EXIT_FAILURE);
}

float small_belt_speed = 0;

// Small belt functions
int belt_small_set_speed(const float speed) {
	small_belt_speed = speed;

	if (small_belt_speed < 0) {
		fprintf(stderr, "[%d] Moving small belt to the inside\n", get_own_id());
	} else if (small_belt_speed > 0) {
		fprintf(stderr, "[%d] Moving small belt to the outside\n", get_own_id());
	} else {
		fprintf(stderr, "[%d] Stopping small belt\n", get_own_id());
	}

	return (int)speed;
}

float belt_small_get_speed() { return small_belt_speed; }

// Tests: methods not used
int64_t belt_small_get_encoder_count() {
	fprintf(stderr, "Small belt encoder count unimplemented\n");
	exit(EXIT_FAILURE);
}

double belt_small_get_encoder_freq() {
	fprintf(stderr, "Small belt freq unimplemented\n");
	exit(EXIT_FAILURE);
}

float big_belt_speed = 0;

// Big belt functions

int belt_big_set_speed(const float speed) {
	big_belt_speed = speed;

	if (big_belt_speed < 0) {
		fprintf(stderr, "[%d] Moving belt to the right\n", get_own_id());
	} else if (big_belt_speed > 0) {
		fprintf(stderr, "[%d] Moving belt to the left\n", get_own_id());
	} else {
		fprintf(stderr, "[%d] Stopping belt\n", get_own_id());
	}

// #ifdef Q_TEST
// 	sleep(1000);
//
// 	if (big_belt_speed < 0) {
// 		laser_right_detected = true;
// 	} else if (big_belt_speed > 0) {
// 		laser_left_detected = true;
// 	}
// #endif

	return (int)speed;
}

float belt_big_get_speed() { return big_belt_speed; }

// Tests: methods not used
int64_t belt_big_get_encoder_count() {
	fprintf(stderr, "Big belt encoder count unimplemented\n");
	exit(EXIT_FAILURE);
}

double belt_big_get_encoder_freq() {
	fprintf(stderr, "Big belt freq unimplemented\n");
	exit(EXIT_FAILURE);
}

// DIR_RFID functions
int RFID_check_tag() {
	if (recent_rfid[0] == '\0' || recent_test_read.source == (uint8_t) -1) {
		return false;
	}
	if (get_own_id() != TEST_TUB_MODULE && get_own_id() != TEST_PLANE_MODULE) {
		return false;
	}

	fprintf(stderr, "[%d] Read RFID\n", get_own_id());
	memcpy(recent_rfid, recent_test_read.data + sizeof(char), DATA_RFID_LENGTH);

	return true;
}

int RFID_write_data_block(int data_ptr, int offset) {
	fprintf(stderr, "Writing RFID blocks unimplemented\n");
	exit(EXIT_FAILURE);
}

int RFID_read_data_block(char* data_ptr, const int offset) {
	if (offset < 0 || offset >= DATA_RFID_LENGTH) {
		return -1; // invalid offset
	}

	data_ptr[0] = recent_rfid[offset];
	return 0;
}

int RFID_get_uid(int uid_pointer) {
	fprintf(stderr, "RFID get uid unimplemented\n");
	exit(EXIT_FAILURE);
}

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

	printf("[%d] Received packet from %d (size %d) -> ", get_own_id(), packet.source, packet.size);
	for (int i = 0; i < packet.size; ++i) {
		printf("%d ", (int)packet.data[i]);
	}
	printf("\n");

	recent_test_read = (Packet){.source = -1, .type = -1, .size = -1, .data = ""};

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

	const int success = send_queue(id, &packet);

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

mqd_t get_display_queue() {
	return display_queue;
}

void reset_rfid() {
	for (int i = 0; i < DATA_RFID_LENGTH; ++i) {
		recent_rfid[i] = '\0';
	}
}

int last_update = 0;

void update_belt() {

}