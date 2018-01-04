#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <unistd.h>

#include "binding.h"
#include "mapping.h"

int main(int argc, char * const argv[])
{
    if (argc < 2)
    {
        printf("Provide IP address.");
        return 1;
    }

    struct binding bindings[4];

    // Init SDL
    SDL_Init(SDL_INIT_JOYSTICK);

    // Test
    add_gc_mapping(2);

    return 0;
    // END Test

    int err;
    // Pass argv from the first element
    if ((err = bind_controllers((const char * const*) &argv[1], bindings)) != 0)
    {
        printf("Error %d while binding controllers.", err);
        return 1;
    }

    printf("%s", SDL_JoystickName(bindings[0].bd_controller));

    for (;;)
    {
        SDL_JoystickUpdate();
        //unsigned int a_btn = SDL_JoystickGetButton(SDL_GameControllerGetJoystick(controller.bd_controller), 0);
        Sint16 a_btn = SDL_JoystickGetButton(bindings[0].bd_controller, 0);
        printf("%d\n", a_btn);
    }
    return 0;
}
