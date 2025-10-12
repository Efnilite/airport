//
// Created by Rens on 11/09/25.
//

#ifndef TEST_H
#define TEST_H

#include "../src/defs.h"
#include "defs.h"

const TestPlane TEST_PLANE = {

	.id = 1,
	.departure_time = 200,
	.leaving = true,

};

const uint8_t TEST_PLANE_MODULE = 5;

const TestTub TEST_TUB = {

	.id = 0,
	.plane_id = 1,
	.destination_id = -1,
	.destination_type = DEST_PLANE,
	.priority = PRIO_ME,
	.passed_security = false,
	.needs_security = false,
	.plane_dropoff = false,
	.plane_arrived = false,
	.safe = true

};

const uint8_t TEST_TUB_MODULE = 3;

#endif // TEST_H