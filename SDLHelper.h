#ifndef SDL_HELPER_H
#define SDL_HELPER_H

#include <SDL2/SDL.h>
#include "Graphics.h"


#define WINDOW_TITLE  "CoronaGame"
#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 640
#define RENDER_SCALE_FACTOR 2.0

#define TEX_PLAYER "gfx/player.bmp"
#define TEX_BLOCK  "gfx/block.bmp"


typedef enum E_SDL {
	E_SDL_SUCCESS = 0,
	E_SDL_INIT_VIDEO,
	E_SDL_TEX_FILTER,
	E_SDL_WINDOW,
	E_SDL_RENDERER,
} E_SDL;

SDL_Window*   sdl_window;
SDL_Renderer* sdl_renderer;

E_SDL SDLHelper_init(void);
void  SDLHelper_quit(void);

SDL_Texture* SDLHelper_load_texture(const char* path);
SDL_Rect     SDLHelper_get_sdl_rect(const Rectangle* rect);


#endif
