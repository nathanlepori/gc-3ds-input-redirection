#ifndef GC_CONTROLLER_H_INCLUDED
#define GC_CONTROLLER_H_INCLUDED

#include <SDL2/SDL.h>

SDL_Joystick *joystick_from_port(int port_num);

SDL_GameController *controller_from_port(int port_num);

#endif // GC_CONTROLLER_H_INCLUDED
