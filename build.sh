#!/bin/bash

clang main.c Game.c Util.c Level.c Entity.c Graphics.c SDLHelper.c ent/Player.c ent/Enemy.c ent/Text.c log/Logger.c -lSDL2 -lm

