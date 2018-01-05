#ifndef BINDING_H_INCLUDED
#define BINDING_H_INCLUDED

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <SDL.h>

struct gc_3ds_binding
{
    SDL_GameController *bd_controller;
    struct sockaddr_in bd_3ds_addr;
};

int bind_gc_controller(const char *addr, int port_num, struct gc_3ds_binding * const bd);

int bind_all_gc_controllers(const char * const addrs[], struct gc_3ds_binding bds[]);

#endif // BINDING_H_INCLUDED
