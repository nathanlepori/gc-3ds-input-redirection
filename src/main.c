#include <stdio.h>
#include <SDL2/SDL.h>
#include <errno.h>

#include "../include/binding.h"
#include "../include/redirection.h"
#include "addrslist.h"

int main(int argc, char *const argv[]) {
    // Addresses from list file
    const char *addrs[4];
    int c_addrs = read_addrs(addrs);

    if (c_addrs < 0) {
        fprintf(stderr, "Could not read bindings list file: %s\n", strerror(errno));
        return 1;
    } else if (c_addrs == 0) {
        fprintf(stderr, "No binding IP defined. Exiting.");
        return 1;
    }

    // Init SDL. Video is required for events (dunno why)
    if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0) {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    // Add mapping
    add_gc_mapping();

    // Shutdown SDL when exiting

    // Structures holding constrollers' bindings
    struct gc_3ds_binding *bds[c_addrs];
    memset(bds, 0, sizeof(bds));

    // Initial binding needed since events for controllers already plugged in don't get fired
    int res = bind_controllers(addrs, bds, c_addrs);
    if (res == 0) {
        printf("No controller found.\n");
    } else if (res > 0) {
        printf("%u controller(s) connected.\n", res);
    } else if (res == -1) {
        fprintf(stderr, "Invalid IP format. Check the list file.");
        return 1;
    }

    // Main logic
    if (start_input_loop(bds, addrs) == -1) {
        fprintf(stderr, "Error while creating socket: %s\n", strerror(errno));
    }

    // Free bindings array
    free_bindings(bds);
    // Free addresses array
    free_addrs(addrs);
    // Shutdown SDL
    SDL_Quit();
    return 0;
}
