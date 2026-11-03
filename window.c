#define WINDOW_C

#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

#include "window.h"

static SDL_Window* createWindow(int width, int height);
static SDL_Renderer* createRenderer(SDL_Window *window);
static SDL_Texture* createTexture(SDL_Renderer *renderer, int width, int height);

void initSDL() {
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
		fprintf(stderr, "Error al inicializar SDL2: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
}

void initDisplay(struct Display *display, int width, int height) {
	display->window = createWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
	display->renderer = createRenderer(display->window);
	display->texture = createTexture(display->renderer, width, height);
	
	display->dispWidth = width;
	display->dispHeight = height;
}

void destroyDisplay(struct Display *display) {
	SDL_DestroyTexture(display->texture);
	SDL_DestroyRenderer(display->renderer);
	SDL_DestroyWindow(display->window);

	display->texture = NULL;
	display->renderer = NULL;
	display->window = NULL;
}

void draw(struct Display *display, uint8_t *pixels) {
	int nPixels = display->dispHeight * display->dispWidth;
	uint8_t buffer[nPixels];
	for(int i = 0; i < nPixels; i++) buffer[i] = pixels[i] ? 0xff : 0x00;
	int pitch = display->dispWidth * sizeof(uint8_t);
	SDL_UpdateTexture(display->texture, NULL, buffer, pitch);
	SDL_RenderCopy(display->renderer, display->texture, NULL, NULL);
	SDL_RenderPresent(display->renderer);
}

static SDL_Window* createWindow(int width, int height) {
	SDL_Window *window = SDL_CreateWindow(
		"CHIP-8 Emulator",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width, height,
		SDL_WINDOW_SHOWN
	);

	if(!window) {
		fprintf(stderr, "Error al crear la ventana: %s\n", SDL_GetError());
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	return window;
}

static SDL_Renderer* createRenderer(SDL_Window *window) {
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_RenderSetLogicalSize(renderer, 64, 32);
	return renderer;
}

static SDL_Texture* createTexture(SDL_Renderer *renderer, int width, int height) {
	SDL_Texture *texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_RGB332,
		SDL_TEXTUREACCESS_STREAMING,
		width, height
	);

	return texture;
}