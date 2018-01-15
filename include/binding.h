#ifndef BINDING_H_INCLUDED
#define BINDING_H_INCLUDED

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <SDL2/SDL.h>

struct gc_3ds_binding
{
    SDL_GameController *gc_controller;
    struct sockaddr_in _3ds_addr;
};

int bind_controller(const char *addr, int port_num, struct gc_3ds_binding *const bd);

int bind_all_controllers(const char *const *addrs, struct gc_3ds_binding *bds);

#endif // BINDING_H_INCLUDED
