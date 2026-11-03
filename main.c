#include <SDL2/SDL.h>
#include <stdbool.h>

#include "window.h"
#include "cpu.h"
#include "input.h"

#define TARGET_FPS 60
#define FRAME_TIME_MS (1000 / TARGET_FPS)
#define CYCLES_PER_FRAME 10

int main(int argc, char* argv[]) {
	if(argc != 2) {
		fprintf(stderr, "Uso: ./chip-8 <Juego>\n");
		exit(EXIT_FAILURE);
	}

	char *game = argv[1];

	initSDL();
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

	struct Display display;
	initDisplay(&display, DISPLAY_WIDTH, DISPLAY_HEIGHT);

	struct CPU cpu;
	initCPU(&cpu);
	if(loadProgram(&cpu, game) != 0) {
		exit(EXIT_FAILURE);
	}

	uint32_t lastTime = SDL_GetTicks();
	bool running = true;
	while (running) {
		uint32_t currTime = SDL_GetTicks();
		uint32_t elapsed = currTime - lastTime;

		if(elapsed >= FRAME_TIME_MS) {
			lastTime = currTime;

			for(int i = 0; i < CYCLES_PER_FRAME; i++) {
				emulateCycle(&cpu);
			}

			updateTimers(&cpu);

			if(cpu.drawFlag) draw(&display, (uint8_t *)(&cpu.display));
	
			running = getInput((uint8_t *)(&cpu.key));
		} else {
			SDL_Delay(1);
		}

	}

	destroyDisplay(&display);

	SDL_Quit();

	return 0;
}