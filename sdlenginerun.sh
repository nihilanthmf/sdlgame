#!/bin/bash
gcc sdlengine.c -I/opt/homebrew/Cellar/sdl2/2.32.8/include -L/opt/homebrew/Cellar/sdl2/2.32.8/lib -lSDL2 -o game.out
./game.out