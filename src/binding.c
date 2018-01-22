#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <SDL2/SDL.h>

#include "../include/binding.h"
#include "../include/gc_controller.h"

#define BD_3DS_PORT 4950

//#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/*
 * Returns 0 on success, 1 on controller error and -1 on address error.
 */
int bind_controller(const char *addr, int port_num, struct gc_3ds_binding *bd)
{
    SDL_GameController *gc = controller_from_port(port_num);
    if (!gc)
    {
        return 1;
    }
    bd->gc_controller = gc;

    // Temp variable
    struct sockaddr_in sockaddr;
    // Clear memory
    memset(&sockaddr, 0, sizeof(sockaddr));
    // Sending over internet
    sockaddr.sin_family = AF_INET;
    // Set port
    sockaddr.sin_port = htons(BD_3DS_PORT);

    // Try to convert IP address and return 1 in case of failure
    if (inet_pton(AF_INET, addr, &sockaddr.sin_addr) != 1)
    {
        return -1;
    }

    // Save reference
    bd->_3ds_addr = sockaddr;

    return 0;
}

/*
 * Same as bind_controller. Warning: it does not check if device is a GC controller.
 */
int bind_controller_from_index(const char *addr, int device_index, struct gc_3ds_binding *bd) {
    SDL_GameController *gc = SDL_GameControllerOpen(device_index);
    if (!gc) {
        return 1;
    }
    bd->gc_controller = gc;

    // Temp variable
    struct sockaddr_in sockaddr;
    // Clear memory
    memset(&sockaddr, 0, sizeof(sockaddr));
    // Sending over internet
    sockaddr.sin_family = AF_INET;
    // Set port
    sockaddr.sin_port = htons(BD_3DS_PORT);

    // Try to convert IP address and return 1 in case of failure
    if (inet_pton(AF_INET, addr, &sockaddr.sin_addr) != 1) {
        return -1;
    }

    // Save reference
    bd->_3ds_addr = sockaddr;

    return 0;
}

struct gc_3ds_binding *binding_from_gc_id(int iid, struct gc_3ds_binding *bds[]) {
    for (int i = 0; i < 4; ++i) {
        if (!bds[i]) {
            continue;
        }
        SDL_GameController *gc = SDL_GameControllerFromInstanceID(iid);
        if (bds[i]->gc_controller == gc) {
            return bds[i];
        }
    }
    return NULL;
}

int bind_controllers(const char *addrs[], struct gc_3ds_binding *bds[], int num_bds) {
    // Reset pointers
    memset(bds, 0, sizeof(struct gc_3ds_binding *) * 4);

    // No joystick connected, return error
    if (SDL_NumJoysticks() == 0)
    {
        return 0;
    }

    // Count of controllers bound so far
    int c_bound = 0;
    // Loop over the controllers
    for (int i = 0; i < 4; i++)
    {
        // If there are no more addresses left, just exit loop
        if (c_bound >= num_bds) {
            break;
        }
        // Allocate space for the binding
        struct gc_3ds_binding *bd = malloc(sizeof(struct gc_3ds_binding));

        // Try to bind
        int err = bind_controller(addrs[c_bound], i, bd);
        // Check errors
        if (err == -1)
        {
            // Free pointer since it's not used anymore
            free(bd);
            // In case of IP format error return error code
            return -1;
        } else if (err == 1) {
            // Free pointer since it's not used anymore
            free(bd);
            // Controller not found: just skip the port
            continue;
        }
        // Print some information
        printf("Port %u -> %s: connected\n", i, addrs[c_bound]);
        // If function returns 0, binding is successful
        bds[c_bound++] = bd;
    }

    // Number of connections on success
    return c_bound;
}

int next_binding_index(struct gc_3ds_binding *bds[]) {
    for (int i = 0; i < 4; ++i) {
        if (!bds[i]) {
            return i;
        }
    }
    return -1;
}

/*
 * Returns -1 on address error, 1 if there is no available address and 0 on success.
 */
int next_addr(const char *addrs[], struct gc_3ds_binding *bds[], char *addr) {
    for (int i = 0; i < 4; ++i) {
        if (!addrs[i]) {
            break;
        }
        // Flag which gets set when an address is found
        bool found = false;
        for (int j = 0; j < 4; ++j) {
            // Skip if empty
            if (!bds[j]) {
                continue;
            }
            // Convert address to string
            char a[INET_ADDRSTRLEN] = "";
            if (!inet_ntop(bds[j]->_3ds_addr.sin_family, &(bds[j]->_3ds_addr.sin_addr), a, sizeof(a))) {
                // This doesn't really matter, but still catch error
                return -1;
            }
            if (!strcmp(a, addrs[i])) {
                found = true;
                break;
            }
        }
        // If found go to the next iteration
        if (found) {
            continue;
        }
        // If not found, copy into the buffer and return positive result
        strcpy(addr, addrs[i]);
        return 0;
    }
    return 1;
}

int bind_next_controller(const char *addrs[], struct gc_3ds_binding *bds[], int device_index) {
    // Check if the new controller is a GC one
    if (!is_gc_controller(device_index)) {
        return -1;
    }
    // Get next free index in bindings structure
    int bdi = next_binding_index(bds);
    // No more bindings available
    if (bdi == -1) {
        return -1;
    }
    char addr[INET_ADDRSTRLEN];
    if (next_addr(addrs, bds, addr)) {
        return -1;
    }

    // Allocate space for the binding
    bds[bdi] = malloc(sizeof(struct gc_3ds_binding));

    bind_controller_from_index(addr, device_index, bds[bdi]);

    // Print some information
    printf("Port %u -> %s: connected\n", controller_port_from_index(device_index), addr);

    return 0;
}

bool remove_controller(struct gc_3ds_binding *bds[], int inst_id) {
    SDL_GameController *gc = SDL_GameControllerFromInstanceID(inst_id);

    for (int i = 0; i < 3; ++i) {
        if (!bds[i]) {
            continue;
        }
        if (bds[i]->gc_controller == gc) {
            char addr[INET_ADDRSTRLEN] = "";
            // Convert address to string
            if (!inet_ntop(bds[i]->_3ds_addr.sin_family, &(bds[i]->_3ds_addr.sin_addr), addr, sizeof(addr))) {
                // This doesn't really matter, but still catch error
                return false;
            }

            // Print some info
            printf("Port %u -> %s: disconnected\n", controller_port_from_id(inst_id), addr);

            free(bds[i]);
            bds[i] = NULL;
            return true;
        }
    }

    return false;
}

void free_bindings(struct gc_3ds_binding *bds[]) {
    for (int i = 0; i < 4; ++i) {
        if (bds[i]) {
            free(bds[i]);
        }
    }
}
