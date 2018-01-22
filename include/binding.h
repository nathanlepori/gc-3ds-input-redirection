#ifndef BINDING_H_INCLUDED
#define BINDING_H_INCLUDED

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

struct gc_3ds_binding
{
    SDL_GameController *gc_controller;
    struct sockaddr_in _3ds_addr;
};

struct gc_3ds_binding *binding_from_gc_id(int iid, struct gc_3ds_binding *bds[]);

int bind_controller(const char *addr, int port_num, struct gc_3ds_binding *bd);

int bind_controllers(const char *addrs[], struct gc_3ds_binding *bds[], int num_bds);

int bind_next_controller(const char *addrs[], struct gc_3ds_binding *bds[], int device_index);

bool remove_controller(struct gc_3ds_binding *bds[], int inst_id);

void free_bindings(struct gc_3ds_binding *bds[]);

#endif // BINDING_H_INCLUDED
