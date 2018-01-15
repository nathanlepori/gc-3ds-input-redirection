#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <errno.h>

#include "../include/binding.h"
#include "../include/redirection.h"
#include "../include/config.h"

int main(int argc, char * const argv[])
{
    // Allocate space up to 4 IP addresses as strings
    const char **addrs = malloc(16 * 4);
    int c_addrs;
    if (argc < 2)
    {
        // Read from config file
        c_addrs = read_config_addrs(addrs);
        if (c_addrs == -1) {
            printf("Couldn't read config file. Error msg: %s\n", strerror(errno));
            return 1;
        }
    } else {
        c_addrs = argc - 1;
        addrs = (const char **) &argv[1];
    }

    struct gc_3ds_binding bds[4];
    memset(bds, 0, sizeof(bds));

    // Init SDL
    SDL_Init(SDL_INIT_GAMECONTROLLER);

    if (bind_all_controllers(addrs, bds))
    {
        printf("No controller found.\n");
        return 1;
    }

    printf("%u redirect(s) bound. Entering input loop.\n", c_addrs);
    //start_binding_input_loop(&bds[0], 0);
    if (start_input_loop((const struct gc_3ds_binding **) &bds, c_addrs, 0) == -1)
    {
        printf("Could not connect. Error msg: %s\n", strerror(errno));
        return 1;
    }

    return 0;
}
