#include <SDL2/SDL.h>
#include <math.h>

#include "SDLHelper.h"


E_SDL SDLHelper_init(void) 
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("[SDL] could not initialize. error: %s\n", SDL_GetError());
		return E_SDL_INIT_VIDEO;
	}

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0")) {
		printf("[SDL] linear texture filtering enabled.\n");
	}

	sdl_window = SDL_CreateWindow(
		WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
		WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (sdl_window == NULL) {
		printf("[SDL] window could not be created. error: %s\n", SDL_GetError());
		return E_SDL_WINDOW;
	}

	sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
	if (sdl_renderer == NULL) {
		printf("[SDL] renderer could not be created. error: %s\n", SDL_GetError());
		return E_SDL_RENDERER;
	}

	printf("[SDL] initialized\n");
	SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderSetScale(sdl_renderer, RENDER_SCALE_FACTOR, RENDER_SCALE_FACTOR);

	return E_SDL_SUCCESS;
}


void SDLHelper_quit(void)
{
	SDL_DestroyRenderer(sdl_renderer);
	SDL_DestroyWindow(sdl_window);
	sdl_renderer = NULL;
	sdl_window = NULL;

	SDL_Quit();
	printf("[SDL] quit\n");
}

SDL_Texture* SDLHelper_load_texture(const char* path)
{
	SDL_Surface* surface = SDL_LoadBMP(path);
	if (surface == NULL) {
		printf("[SDL] unable to load image %s. error: %s\n", path, SDL_GetError());
		return NULL;
	} 

    SDL_Texture* texture = SDL_CreateTextureFromSurface(sdl_renderer, surface);
	if (texture == NULL) {
		printf("[SDL] unable to create texture from %s. error: %s\n", path, SDL_GetError());
		return NULL;
	}
	SDL_FreeSurface(surface);

	printf("[SDL] loaded image and created texture from %s.\n", path);
	return texture;
}

SDL_Rect SDLHelper_get_sdl_rect(const Rectangle* rect) 
{
	if (!rect) {
		return (SDL_Rect) {
			.x = -1,
			.y = -1,
			.w = 0,
			.h = 0
		};
	}

	SDL_Rect sdl_rect = {
		.x = round(rect->position.x),
		.y = round(rect->position.y),
		.w = round(rect->size.x),
		.h = round(rect->size.y)
	};
	return sdl_rect;
}
