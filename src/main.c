#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <unistd.h>
#include <errno.h>

#include "binding.h"
#include "mapping.h"
#include "gc_controller.h"
#include "redirection.h"

int main(int argc, char * const argv[])
{
    if (argc < 2)
    {
        printf("Provide IP address.");
        return 1;
    }

    struct gc_3ds_binding bds[4];
    memset(bds, 0, sizeof(bds));

    // Init SDL
    SDL_Init(SDL_INIT_JOYSTICK);

    if (bind_all_gc_controllers((const char * const*)&argv[1], bds))
    {
        printf("Error while binding controllers.");
        return 1;
    }

    printf("Controller bound. Entering input loop.\n");
    if (gc_input_loop(&bds[0], &default_3ds_mapping, 0) == -1)
    {
        printf("Could not connect. Errno: %d", errno);
    }

    return 0;
}
