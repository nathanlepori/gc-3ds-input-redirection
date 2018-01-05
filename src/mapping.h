#ifndef MAPPING_H_INCLUDED
#define MAPPING_H_INCLUDED

#include <SDL.h>

struct _3ds_mapping
{
    SDL_GameControllerButton a, b, y, x, start, home, select, power, dpup, dpleft, dpdown, dpright;

    SDL_GameControllerAxis leftx, lefty, rightx, righty;

    union
    {
        SDL_GameControllerAxis axis;
        SDL_GameControllerButton button;
    } zl, zr, l, r;
} const default_3ds_mapping;

int add_gc_mapping(int port_num);

#endif // MAPPING_H_INCLUDED
