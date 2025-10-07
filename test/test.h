//
// Created by Rens on 11/09/25.
//

#ifndef TEST_H
#define TEST_H

#include "../src/defs.h"

#include <stdint.h>

/**
 * The two belts present on every module.
 */
typedef enum { SMALL_BELT, BIG_BELT } Belt;

/**
 * The two lasers present on every module.
 */
typedef enum { LEFT_LASER, RIGHT_LASER } Laser;

void place_tub(uint8_t module, const Tub* tub);

#endif // TEST_H
