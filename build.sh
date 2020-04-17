#!/bin/bash

clang -Werror -Wall main.c Game.c Util.c Level.c Viewport.c Entity.c Graphics.c SDLHelper.c ent/Player.c ent/Enemy.c ent/Text.c log/Logger.c -lSDL2 -lm

