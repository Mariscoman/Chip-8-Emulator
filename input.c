#define INPUT_C

#include <SDL2/SDL.h>
#include "cpu.h"
#include <stdint.h>
#include <stdbool.h>

#include "input.h"

static int simToRealKeyMapping(SDL_Scancode scancode) {
	switch(scancode) {
		case SDL_SCANCODE_1: return 0x1;
		case SDL_SCANCODE_2: return 0x2;
		case SDL_SCANCODE_3: return 0x3;
		case SDL_SCANCODE_4: return 0xc;
		case SDL_SCANCODE_Q: return 0x4;
		case SDL_SCANCODE_W: return 0x5;
		case SDL_SCANCODE_E: return 0x6;
		case SDL_SCANCODE_R: return 0xd;
		case SDL_SCANCODE_A: return 0x7;
		case SDL_SCANCODE_S: return 0x8;
		case SDL_SCANCODE_D: return 0x9;
		case SDL_SCANCODE_F: return 0xe;
		case SDL_SCANCODE_Z: return 0xa;
		case SDL_SCANCODE_X: return 0x0;
		case SDL_SCANCODE_C: return 0xb;
		case SDL_SCANCODE_V: return 0xf;
		default: return -1;
	}
}

bool getInput(uint8_t *keys) {
	SDL_Event evt;

	while(SDL_PollEvent(&evt)) {
		if(evt.type == SDL_QUIT) return false;

		if(evt.type == SDL_KEYDOWN) {
			if(evt.key.repeat != 0) continue;

			int key = simToRealKeyMapping(evt.key.keysym.scancode);
			if(key != -1) keys[key] = KEY_DOWN;
		} else if(evt.type == SDL_KEYUP) {
			int key = simToRealKeyMapping(evt.key.keysym.scancode);
			if(key != -1) keys[key] = KEY_UP;
		}
	}

	return true;
}