#!/bin/bash

clang -Werror -Wall -ggdb3 main.c Game.c Util.c World.c Level.c Viewport.c Entity.c Graphics.c SDLHelper.c ent/Player.c ent/Enemy.c ent/Text.c Log.c -lSDL2 -lm

