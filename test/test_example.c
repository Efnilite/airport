//
// Created by Rens on 5/09/25.
//

#include "../src/defs.h"
#include "test.h"

void test_example() {
	const Tub tub = {
		 .id = 0,
		 .plane_id = 1,
		 .destination_id = 13,
		 .destination_type = DEST_SECURITY,
		 .priority = PRIO_ME,
		 .passed_security = false,
		 .plane_dropoff = true,
		 .plane_arrived = false
	};

	// start tub simulation
	place_tub(13, &tub);

	// transfer tub to new module
	// transfer_tub(13, 5, &tub);

	// continue for every module...
	// very rough draft ofc!
}
