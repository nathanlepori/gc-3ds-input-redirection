#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <SDL.h>

#include <regex.h>
#include <stdlib.h>

#include "binding.h"
#include "gc_controller.h"

#define BD_3DS_PORT 4950

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/*
 * Returns 0 on success, 1 on controller error and -1 on address error.
 */
int bind_gc_controller(const char *addr, int port_num, struct gc_3ds_binding * const bd)
{
    SDL_GameController *gc = controller_from_port(port_num);
    if (!gc)
    {
        return 1;
    }
    bd->bd_controller = gc;

    // Temp variable
    struct sockaddr_in sockaddr;
    // Clear memory
    memset(&sockaddr, 0, sizeof(sockaddr));
    // Sending over internet
    sockaddr.sin_family = AF_INET;
    // Set port
    sockaddr.sin_port = htons(BD_3DS_PORT);

    // Try to convert IP address and return 1 in case of failure
    if (inet_pton(AF_INET, addr, &(sockaddr.sin_addr)) != 1)
    {
        return -1;
    }

    // Save reference
    bd->bd_3ds_addr = sockaddr;

    return 0;
}

int bind_all_gc_controllers(const char * const addrs[], struct gc_3ds_binding bds[])
{
    // No joystick connected, return error
    if (SDL_NumJoysticks() == 0)
    {
        return -1;
    }

    // Count of controllers binded so far
    int c_bound = 0;
    // Loop over the controllers
    for (int i = 1; i <= 4; i++)
    {
        // Store the temporary binding
        struct gc_3ds_binding bd;
        // Try to bind
        int err = bind_gc_controller(addrs[c_bound], i, &bd);
        // If function return 0 binding is successful
        if (err == 0)
        {
            bds[c_bound++] = bd;
        } else if (err == -1)
        {
            // In case of IP format error return error code
            return err;
        }
    }
    if (c_bound == 0)
    {
        return 1;
    }

    // Null on success
    return 0;
}
