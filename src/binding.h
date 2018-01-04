#ifndef BINDING_H_INCLUDED
#define BINDING_H_INCLUDED

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <SDL.h>

struct binding
{
    struct sockaddr_in bd_3ds_addr;
    SDL_Joystick *bd_controller;
};

int bind_controller(const char *addr, int joystic_index, struct binding * const bd);

int bind_controllers(const char * const addrs[], struct binding bds[]);

#endif // BINDING_H_INCLUDED
