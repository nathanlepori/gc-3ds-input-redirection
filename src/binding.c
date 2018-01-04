#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <SDL.h>

#include <regex.h>
#include <stdlib.h>

#include "binding.h"

#define BD_NUM_MAX_CONTROLLERS 4
#define BD_3DS_PORT 4950

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

int bind_controllers(const char * const addrs[], struct binding bds[])
{
    if (SDL_NumJoysticks() == 0)
    {
        return -1;
    }

    // Get connected controllers (clamped)
    int numJoy = MIN(SDL_NumJoysticks(), BD_NUM_MAX_CONTROLLERS);

    // Loop over the controllers
    for (int i = 0; i < numJoy; i++)
    {
        int err;
        if ((err = bind_controller(addrs[i], i, &bds[i])))
        {
            // Forward the error
            return err;
        }
    }

    // Null on success
    return 0;
}

/*
 * Returns 0 on success, 1 on controller error and -1 on address error.
 */
int bind_controller(const char *addr, int joystic_index, struct binding * const bd)
{
    SDL_Joystick *joy = SDL_JoystickOpen(joystic_index);
    if (joy == NULL)
    {
        return 1;
    }

    // Get controller
    bd->bd_controller = joy;

    // Temp variable
    struct sockaddr_in sockaddr;
    // Clear memory
    memset(&sockaddr, 0, sizeof(sockaddr));
    // Sending over internet
    sockaddr.sin_family = AF_INET;
    // Set port
    sockaddr.sin_port = htons(BD_3DS_PORT);

    // Try to convert IP address and return 1 in case of failure
    if (inet_pton(AF_INET, addr, &(sockaddr.sin_addr)))
    {
        return -1;
    }

    // Save reference
    bd->bd_3ds_addr = sockaddr;

    return 0;
}

/*
 * Returns 0 on success, 1 on controller error and -1 on address error.
 */
int bind_gc_controller(const char *addr, int port_num, struct binding * const bd)
{
    // Temp variable
    struct sockaddr_in sockaddr;
    // Clear memory
    memset(&sockaddr, 0, sizeof(sockaddr));
    // Sending over internet
    sockaddr.sin_family = AF_INET;
    // Set port
    sockaddr.sin_port = htons(BD_3DS_PORT);

    // Try to convert IP address and return 1 in case of failure
    if (inet_pton(AF_INET, addr, &(sockaddr.sin_addr)))
    {
        return 1;
    }

    // Save reference
    bd->bd_3ds_addr = sockaddr;

    return 0;
}
