//
// Created by efy on 7/10/25.
//

#include "../defs.h"
#include "../queue.h"
#include "display.h"

#include <mqueue.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

/**
 * Parses a number from a regex result.
 *
 * @param matches The matched regex value.
 * @param line The full line.
 * @return The parsed integer, or `(unsigned int) -1` if the value is invalid.
 */
unsigned int parse_regex(const regmatch_t matches, char line[100]) {
	const int valLen = matches.rm_eo - matches.rm_so;
	char val[valLen + 1];
	strncpy(val, line + matches.rm_so, valLen);
	val[valLen] = '\0';

	char* end_str;
	const long value = strtol(val, &end_str, 10);
	if (end_str == val) {
		return -1;
	}

	return value;
}

/**
 * Represents all neighbors a module may have.
 */
typedef struct {
	unsigned int left;
	unsigned int right;
	unsigned int bottom;
} Neighbors;

/**
 * Parses the network map file to allow viewing display.
 * Populates neighbors.
 */
void parse_map(Neighbors* neighbors) {
	FILE* file = fopen("../network_map", "r");

	if (file == NULL) {
		perror("Failed to read network_map");
		exit(EXIT_FAILURE);
	}

	regex_t regex;
	regmatch_t matches[5]; // full match + 4 groups

	// M9,tdefault,a10,b13,c8
	const int regex_result = regcomp(&regex, "M([0-9]+),t[a-z-]+,a([0-9]+),b([0-9]+),c([0-9]+)", REG_EXTENDED);

	if (regex_result != 0) {
		perror("Failed to compile regex");
		fclose(file);
		exit(EXIT_FAILURE);
	}

	char line[100];

	// file version. unused.
	fgets(line, sizeof(line), file);

	while (true) {
		const char* read = fgets(line, sizeof(line), file);
		if (read == NULL) {
			break;
		}

		if (regexec(&regex, line, 5, matches, 0) != 0) {
			fprintf(stderr, "Line does not match network_map format: %s\n", line);
			regfree(&regex);
			fclose(file);
			exit(EXIT_FAILURE);
		}

		const unsigned int id = parse_regex(matches[1], line);

		// neighbors are assigned left -> bottom -> right if module is shaped like a T.
		neighbors[id] = (Neighbors) {
			.left = parse_regex(matches[2], line),
			.bottom = parse_regex(matches[3], line),
			.right = parse_regex(matches[4], line)
		};
	}

	regfree(&regex);
	fclose(file);
}

typedef struct {
	int x;
	int y;
} Position;

void calculate_positions(const Neighbors* neighbors, Position* positions) {
	positions[CENTER_MODULE_ID] = (Position) {
		.x = 200,
		.y = 200
	};

	// the current id.
	unsigned int current_id = CENTER_MODULE_ID;
	// all current neighbors.
	Neighbors current_neighbors = neighbors[CENTER_MODULE_ID];

	// represents all nodes that have been visited.
	bool visited[MAX_PROCESSES];
	// represents all nodes that have been found. used to get out of dead ends.
	bool found[MAX_PROCESSES];

	for (int i = 0; i < MAX_PROCESSES; ++i) {
		visited[i] = false;
		found[i] = false;
	}
	visited[0] = true;
	visited[9] = true;

	while (true) {
start:

		if (!visited[current_neighbors.bottom]) {
			const Position new = (Position) {
				.x = positions[current_id].x,
				.y = positions[current_id].y + MODULE_SIZE_PX,
			};
			current_id = current_neighbors.bottom;
			positions[current_id] = new;
		} else if (!visited[current_neighbors.left]) {
			const Position new = (Position) {
				.x = positions[current_id].x - MODULE_SIZE_PX,
				.y = positions[current_id].y,
			};
			current_id = current_neighbors.left;
			positions[current_id] = new;
		} else if (!visited[current_neighbors.right]) {
			const Position new = (Position) {
				.x = positions[current_id].x + MODULE_SIZE_PX,
				.y = positions[current_id].y,
			};
			current_id = current_neighbors.right;
			positions[current_id] = new;
		} else {
			// all neighbors have been visited
			for (int i = 0; i < MAX_PROCESSES; ++i) {
				if (!visited[i] && found[i]) {
					// find any neighbor to get coords
					for (int j = 0; j < MAX_PROCESSES; ++j) {
						if (found[j]
							&& (neighbors[j].left == i || neighbors[j].bottom == i || neighbors[j].right == i)) {
							current_id = j;
							current_neighbors = neighbors[current_id];
							goto start;
						}
					}
					break;
				}
			}
			break;
		}

		current_neighbors = neighbors[current_id];
		found[current_neighbors.left] = true;
		found[current_neighbors.bottom] = true;
		found[current_neighbors.right] = true;
		visited[current_id] = true;
	}

	for (int i = 0; i < MAX_PROCESSES; ++i) {
		fprintf(stderr, "ID %d gets pos %d %d\n", i, positions[i].x, positions[i].y);
	}
}

/**
 * The id of the display queue.
 */
static mqd_t display_queue;

/**
 * The colors of all modules.
 */
static unsigned long colors[MAX_PROCESSES];

/**
 * The id of the module which has the tub.
 */
static unsigned int tub_id = 0;

/**
 * The position of the tub inside the module.
 */
static unsigned int tub_position = 0;

/**
 * @param r The red value
 * @param g The green value
 * @param b The blue value
 * @return The hex representation of this rgb value
 */
static unsigned long rgb_to_hex(const unsigned int r, const unsigned int g, const unsigned int b) {
	return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

static void update(void) {
	Packet packet;
	const ssize_t size = receive_queue(display_queue, &packet);

	if (size == (ssize_t)-1 || packet.type != TYPE_TEST) {
		return;
	}

	const DisplayPacketType type = (unsigned char)packet.data[0];

	switch (type) {
	case DISPLAY_COLOR_UPDATE:
		const unsigned int r = (unsigned char)packet.data[1];
		const unsigned int g = (unsigned char)packet.data[2];
		const unsigned int b = (unsigned char)packet.data[3];

		colors[packet.source] = rgb_to_hex(r, g, b);
		break;
	case DISPLAY_POSITION_UPDATE:
		tub_id = (unsigned char)packet.data[1];
		tub_position = (unsigned char)packet.data[2];
		break;
	default:
		fprintf(stderr, "Invalid packet type %d", type);
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char* argv[]) {
	Position positions[MAX_PROCESSES];

	// setup
	{
		Neighbors neighbors[MAX_PROCESSES];

		parse_map(neighbors);
		calculate_positions(neighbors, positions);

		display_queue = open_queue("/QA to Display", O_RDONLY | O_CREAT | O_NONBLOCK);
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}
	if (TTF_Init() < 0) {
		fprintf(stderr, "TTF_Init error: %s\n", TTF_GetError());
		SDL_Quit();
		return 1;
	}

	SDL_Window* window =
		SDL_CreateWindow("Module Colors", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 400, 400, 0);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 20);
	if (!font) {
		fprintf(stderr, "Font load error: %s\n", TTF_GetError());
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		TTF_Quit();
		SDL_Quit();
		return 1;
	}

	int running = 1;
	SDL_Event e;
	while (running) {
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) running = 0;
		}

		SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
		SDL_RenderClear(renderer);

		update();

		for (int i = 1; i < MAX_PROCESSES; ++i) {
			const unsigned int color = colors[i];
			const uint8_t r = color >> 16 & 0xFF;
			const uint8_t g = color >> 8 & 0xFF;
			const uint8_t b = color & 0xFF;

			const Position position = positions[i];
			if (position.x == 0 && position.y == 0) {
				break;
			}
			SDL_Rect rect = {position.x, position.y, MODULE_SIZE_PX, MODULE_SIZE_PX};
			SDL_SetRenderDrawColor(renderer, r, g, b, 255);
			SDL_RenderFillRect(renderer, &rect);

			char text[32];
			snprintf(text, sizeof(text), "%d", i);
			const SDL_Color textColor = {255, 255, 255, 255};
			SDL_Surface* surface = TTF_RenderText_Blended(font, text, textColor);
			SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
			SDL_Rect dst = {position.x, position.y, surface->w, surface->h};
			SDL_RenderCopy(renderer, texture, NULL, &dst);
			SDL_FreeSurface(surface);
			SDL_DestroyTexture(texture);
		}

		{
			SDL_Rect rect = {(int)tub_id * 40, 40, 40, 40};
			SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			SDL_RenderFillRect(renderer, &rect);

			char text[32];
			snprintf(text, sizeof(text), "%d", tub_position);
			const SDL_Color textColor = {255, 255, 255, 255};
			SDL_Surface* surface = TTF_RenderText_Blended(font, text, textColor);
			SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
			SDL_Rect dst = {(int)tub_id * 40 + 10, 50, surface->w, surface->h};
			SDL_RenderCopy(renderer, texture, NULL, &dst);
			SDL_FreeSurface(surface);
			SDL_DestroyTexture(texture);
		}

		SDL_RenderPresent(renderer);
		SDL_Delay(16);
	}

	TTF_CloseFont(font);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();
	return 0;
}
