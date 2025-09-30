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

/**
 * Asserts that the servo is set at the specified angle.
 * Servo angles can be found in <code>src/defs.h</code>.
 *
 * @param module The module id.
 * @param angle The value to check the current angle against.
 * @param seconds The amount of seconds that may be taken to update this value.
 */
void assert_servo_angle(uint8_t module, int16_t angle, uint16_t seconds);

/**
 * Asserts that the belt speed is at a certain value within <code>seconds</code> seconds.
 * Belt speeds can be found in <code>src/defs.h</code>.
 * These values are defined as if the orientation of the machine was that of a T-shape.
 * Therefore, the value for belt down would be the small belt going to the outside, and
 * belt right would go to the right laser.
 *
 * @param module The module id.
 * @param belt The belt to check the speed for.
 * @param speed The speed to check.
 * @param seconds The amount of seconds that may be taken to update this value.
 */
void assert_belt_speed(uint8_t module, Belt belt, int16_t speed, uint16_t seconds);

void notify_laser(uint8_t module, Laser laser);

// void transfer_tub(uint8_t from, uint8_t to, const Tub* tub);

#endif // TEST_H
