#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <unistd.h>

#include "binding.h"
#include "mapping.h"
#include "gc_controller.h"

int main(int argc, char * const argv[])
{
    if (argc < 2)
    {
        printf("Provide IP address.");
        return 1;
    }

    struct gc_3ds_binding bindings[4];
    memset(bindings, 0, sizeof(bindings));

    // Init SDL
    SDL_Init(SDL_INIT_JOYSTICK);

    int err = bind_all_gc_controllers((const char * const*)&argv[1], bindings);

    return 0;
}
