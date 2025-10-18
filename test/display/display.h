//
// Created by Rens on 16/10/25.
//

#ifndef DISPLAY_H
#define DISPLAY_H

/**
 * The id of the module that should be displayed as the center.
 */
const unsigned int CENTER_MODULE_ID = 9;

/**
 * The size of a module in pixels.
 */
const int MODULE_SIZE_PX = 150;

const int MODULE_SIZE_PX_HALF = MODULE_SIZE_PX / 2;

const int MODULE_GAP = 40;
const int MODULE_EDGE_WIDTH = 20;

const int MODULE_ANGLES[] = {
	0,
	90,
	180,
	180,
	270,
	270,
	90,
	90,
	0,
	0,
	270,
	270,
	90,
	270,
	90,
	0,
	90,
	180,
	270
};

#endif // DISPLAY_H
