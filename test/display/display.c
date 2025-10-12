//
// Created by efy on 7/10/25.
//

#include "../defs.h"
#include "../queue.h"

#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

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
	display_queue = open_queue("/QA to Display", O_RDONLY | O_CREAT | O_NONBLOCK);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF_Init error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Module Colors",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 40 * 20, 200, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font *font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 20);
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

        for (int i = 0; i < MAX_PROCESSES; ++i) {
			const unsigned int color = colors[i];
			const uint8_t r = color >> 16 & 0xFF;
			const uint8_t g = color >> 8 & 0xFF;
			const uint8_t b = color & 0xFF;

            SDL_Rect rect = {i * 40, 0, 40, 40};
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderFillRect(renderer, &rect);

            char text[32];
            snprintf(text, sizeof(text), "%d", i);
			const SDL_Color textColor = {255, 255, 255, 255};
            SDL_Surface *surface = TTF_RenderText_Blended(font, text, textColor);
            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_Rect dst = {i * 40, 0, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, NULL, &dst);
            SDL_FreeSurface(surface);
            SDL_DestroyTexture(texture);
        }

        {
        	SDL_Rect rect = {(int) tub_id * 40, 40, 40, 40};
        	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        	SDL_RenderFillRect(renderer, &rect);

        	char text[32];
        	snprintf(text, sizeof(text), "%d", tub_position);
        	const SDL_Color textColor = {255, 255, 255, 255};
        	SDL_Surface *surface = TTF_RenderText_Blended(font, text, textColor);
        	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        	SDL_Rect dst = {(int) tub_id * 40 + 10, 50, surface->w, surface->h};
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

