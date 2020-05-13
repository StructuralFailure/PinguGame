//
// Created by fabian on 07.05.20.
//

#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "MenuController.h"
#include "../Log.h"
#include "../SDLHelper.h"
#include "../Graphics.h"
#include "../IO.h"
#include "../Viewport.h"

#define BUTTON_WIDTH    64
#define BUTTON_HEIGHT   64
#define BUTTON_PADDING  16


SDL_Texture* tex_buttons;
SDL_Texture* tex_mouse_pointer;


typedef enum MenuButtonType {
	MBT_START,
	MBT_OPTIONS,
	MBT_LEVEL_EDITOR,
	MBT_EXIT
} MenuButtonType;


typedef enum MenuButtonState {
	MBS_DEFAULT,
	MBS_HOVER,
	MBS_PRESSED
} MenuButtonState;


typedef struct MenuButton {
	MenuButtonType type;
	Vector2DInt frame_default;
	Vector2DInt frame_hover;
	Vector2DInt frame_pressed;
	MenuButtonState state;
} MenuButton;


static MenuButton main_menu_buttons[3] = {
		[0] = {
				.type = MBT_START,
				.frame_default = { 0, 0 },
				.frame_hover   = { 0, 1 },
				.frame_pressed = { 0, 2 }
		},
		[1] = {
				.type = MBT_LEVEL_EDITOR,
				.frame_default = { 2, 0 },
				.frame_hover   = { 2, 1 },
				.frame_pressed = { 2, 2 }
		},
		[2] = {
				.type = MBT_EXIT,
				.frame_default = { 1, 0 },
				.frame_hover   = { 1, 1 },
				.frame_pressed = { 1, 2 }
		}
};


#define MAIN_MENU_BUTTON_COUNT (sizeof(main_menu_buttons) / sizeof(MenuButton))


static void update_main_menu(Controller* mc);

static void draw_main_menu(Controller* mc);

bool update_button_state(MenuButton* button, RectangleInt* rect_button);
bool is_in_rect(Vector2DInt* vec, RectangleInt* rect);


Controller* MenuController_create(void)
{
	Log("MenuController_create", "creating.");

	if (!tex_buttons) {
		tex_buttons = SDLHelper_load_texture("assets/gfx/menu.bmp");
	}
	if (!tex_mouse_pointer) {
		tex_mouse_pointer = SDLHelper_load_texture("assets/gfx/mouse_pointer.bmp");
	}

	Controller* controller = Controller_create();
	if (!controller) {
		Log_error("MenuController_create", "failed to create controller.");
		return NULL;
	}
	controller->type = CT_MENU_CONTROLLER;
	controller->draw = MenuController_draw;
	controller->finalize_update = MenuController_finalize_update;
	controller->destroy = MenuController_destroy;

	MenuControllerData* data = calloc(1, sizeof(MenuControllerData));
	if (!data) {
		Log_error("MenuController_create", "failed to allocate memory for data.");
		Controller_destroy(controller);
		return NULL;
	}
	controller->data = data;

	data->state = MCS_MAIN_MENU;
	Log("MenuController_create", "created.");
	return controller;
}


void MenuController_destroy(Controller* mc)
{
	Log("MenuController_destroy", "destroying.");

	free(mc->data);
	free(mc);

	Log("MenuController_destroy", "destroyed.");
}


void MenuController_draw(Controller* mc, Viewport* viewport)
{
	MenuControllerData* data = mc->data;
	if (!data) {
		return;
	}
	switch (data->state) {
	case MCS_MAIN_MENU:
		draw_main_menu(mc);
		break;
	default:;
	}

	Vector2DInt mouse_position = IO_mouse_position();
	SDL_Rect rect_dest = {
			.x = mouse_position.x,
			.y = mouse_position.y,
			.w = 8,
			.h = 8
	};
	SDL_RenderCopy(sdl_renderer, tex_mouse_pointer, NULL, &rect_dest);
}


void MenuController_finalize_update(Controller* mc)
{
	MenuControllerData* data = mc->data;
	if (!data) {
		return;
	}

	switch (data->state) {
	case MCS_MAIN_MENU:
		update_main_menu(mc);
		break;
	default:;
	}
}


#define DRAW_OFFSET_X (                                      \
	VP_DEFAULT_WIDTH / 2 - (                                 \
		  MAIN_MENU_BUTTON_COUNT * BUTTON_WIDTH              \
		+ (MAIN_MENU_BUTTON_COUNT - 1) * BUTTON_PADDING      \
	) / 2                                                    \
)
#define DRAW_OFFSET_Y 96

static void draw_main_menu(Controller* mc)
{
	SDL_Rect rect_dest = {
			.x = DRAW_OFFSET_X,
			.y = DRAW_OFFSET_Y,
			.w = BUTTON_WIDTH,
			.h = BUTTON_HEIGHT
	};

	//Log("MenuController", "button count: %d", MAIN_MENU_BUTTON_COUNT);
	//Log("MenuController", "draw_main_menu_buttons: { x = %d | y = %d | w = %d | h = %d }", rect_dest.x, rect_dest.y, rect_dest.w, rect_dest.h);

	for (int i = 0; i < MAIN_MENU_BUTTON_COUNT; ++i) {
		const MenuButton* button = &(main_menu_buttons[i]);
		Vector2DInt frame;

		switch (button->state) {
		case MBS_DEFAULT:
			frame = button->frame_default;
			break;
		case MBS_HOVER:
			frame = button->frame_hover;
			break;
		case MBS_PRESSED:
			frame = button->frame_pressed;
			break;
		}

		SDL_Rect rect_source = {
				.x = frame.x * BUTTON_WIDTH,
				.y = frame.y * BUTTON_HEIGHT,
				.w = BUTTON_WIDTH,
				.h = BUTTON_HEIGHT
		};
		SDL_RenderCopy(sdl_renderer, tex_buttons, &rect_source, &rect_dest);

		rect_dest.x += BUTTON_WIDTH + BUTTON_PADDING;
	}
}


static void update_main_menu(Controller* mc)
{
	/* check for button hovering. */
	RectangleInt rect_button = {
			.position = { DRAW_OFFSET_X, DRAW_OFFSET_Y },
			.size = { BUTTON_WIDTH, BUTTON_HEIGHT }
	};

	for (int i = 0; i < MAIN_MENU_BUTTON_COUNT; ++i) {
		MenuButton* button = &(main_menu_buttons[i]);
		if (update_button_state(button, &rect_button) && button->type == MBT_START) {
			mc->requested_world = 0; /* first stage. */
			break;
		}
		rect_button.position.x += BUTTON_WIDTH + BUTTON_PADDING;
	}
}


bool update_button_state(MenuButton* button, RectangleInt* rect_button)
{
	Vector2DInt mouse_position = IO_mouse_position();
	bool mouse_left_pressed = IO_mouse_pressed(IO_MOUSE_LEFT);
	bool mouse_left_down = IO_mouse_down(IO_MOUSE_LEFT);

	/* gotta love state machines implemented using switch. */
	switch (button->state) {
	case MBS_DEFAULT:
		if (is_in_rect(&mouse_position, rect_button)) {
			button->state = MBS_HOVER;
		}
		break;
	case MBS_HOVER:
		if (is_in_rect(&mouse_position, rect_button)) {
			if (mouse_left_pressed) {
				button->state = MBS_PRESSED;
			}
		} else {
			button->state = MBS_DEFAULT;
		}
		break;
	case MBS_PRESSED:
		if (!mouse_left_down) {
			if (is_in_rect(&mouse_position, rect_button)) {
				button->state = MBS_HOVER;
			} else {
				button->state = MBS_DEFAULT;
			}
			return true;
		}
		break;
	}
	return false;
}


inline bool is_in_rect(Vector2DInt* vec, RectangleInt* rect)
{
	return (
		vec->x >= rect->position.x &&
		vec->y >= rect->position.y &&
		vec->x < rect->position.x + rect->size.x &&
		vec->y < rect->position.y + rect->size.y
	);
}