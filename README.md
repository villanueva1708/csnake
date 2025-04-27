# csnake
A small snake game for Linux console x86 made in C

BUILDING REQUIREMENTS

apt install gcc libncurses5-dev libsdl2-dev libsdl2-mixer-dev

BUILD

gcc -o csnake csnake.c -lncurses -lSDL2 -lSDL2_mixer
