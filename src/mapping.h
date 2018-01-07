#ifndef MAPPING_H_INCLUDED
#define MAPPING_H_INCLUDED

#include <SDL.h>

#define AXIS_TO_BUTTON_DEADZONE 8000

struct _3ds_mapping
{
    SDL_GameControllerButton a, b, y, x, start, home, select, power, dpup, dpleft, dpdown, dpright;

    /*
     * m_type: 0 = axis; 1 = button
     */
    struct {
        int m_type;
        union
        {
            SDL_GameControllerAxis axis;
            SDL_GameControllerButton button;
        } m;
    } zl, zr, l, r;

    SDL_GameControllerAxis cpadx, cpady, cstickx, csticky;
} const default_3ds_mapping;

int add_gc_mapping(int port_num);

#endif // MAPPING_H_INCLUDED
