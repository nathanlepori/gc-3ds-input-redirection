#ifndef GC_CONTROLLER_H_INCLUDED
#define GC_CONTROLLER_H_INCLUDED

#include <SDL2/SDL.h>
#include <stdbool.h>

int controller_port_from_index(int device_index);

int controller_port_from_id(int inst_id);

bool is_gc_controller(int device_index);

SDL_Joystick *joystick_from_port(int port_num);

SDL_GameController *controller_from_port(int port_num);

#endif // GC_CONTROLLER_H_INCLUDED
