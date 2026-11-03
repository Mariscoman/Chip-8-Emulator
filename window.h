#ifndef WINDOW_H
#define WINDOW_H

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 320

struct Display {
	int dispWidth, dispHeight;

	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
};

void initSDL();
void initDisplay(struct Display *display, int width, int height);
void destroyDisplay(struct Display *display);

void draw(struct Display *display, uint8_t *pixels);

#endif