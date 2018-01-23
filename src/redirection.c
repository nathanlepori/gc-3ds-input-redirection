#include <SDL2/SDL.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "../include/redirection.h"

#define CPAD_MAX 0x5d0
#define CSTICK_MAX 0x7f

// TODO: Implement GC sticks dead zone to send "free" data to 3DS sticks.
#define GC_STICK_DEADZONE 3000
#define GC_TRIGGER_DEADZONE 3000

const struct _3ds_input_state *get_gc_input(SDL_GameController *gc) {
    const struct _3ds_mapping *mp = &default_3ds_mapping;

    // Update the controllers
    SDL_GameControllerUpdate();

    struct _3ds_input_state *is = malloc(sizeof(struct _3ds_input_state));
    memset(is, 0, sizeof(*is));

    is->a = SDL_GameControllerGetButton(gc, mp->a);
    is->b = SDL_GameControllerGetButton(gc, mp->b);
    is->select = SDL_GameControllerGetButton(gc, mp->select);
    is->start = SDL_GameControllerGetButton(gc, mp->start);
    is->dpright = SDL_GameControllerGetButton(gc, mp->dpright);
    is->dpleft = SDL_GameControllerGetButton(gc, mp->dpleft);
    is->dpup = SDL_GameControllerGetButton(gc, mp->dpup);
    is->dpdown = SDL_GameControllerGetButton(gc, mp->dpdown);
    // R
    if (mp->r.m_type) {
        // Button
        is->r = SDL_GameControllerGetButton(gc, mp->r.m.button);
    } else {
        // Axis
        is->r = (uint8_t) (SDL_GameControllerGetAxis(gc, mp->r.m.axis) >= TRIGGER_TO_BUTTON_DEADZONE ? 1 : 0);
    }
    // L
    if (mp->l.m_type) {
        // Button
        is->l = SDL_GameControllerGetButton(gc, mp->l.m.button);
    } else {
        // Axis
        is->l = (uint8_t) (SDL_GameControllerGetAxis(gc, mp->l.m.axis) >= TRIGGER_TO_BUTTON_DEADZONE ? 1 : 0);
    }
    is->x = SDL_GameControllerGetButton(gc, mp->x);
    is->y = SDL_GameControllerGetButton(gc, mp->y);

    // Circle pad
    // 0x8000 = SDL max axis value
    int16_t ctx = SDL_GameControllerGetAxis(gc, mp->cpadx);
    int16_t cty = SDL_GameControllerGetAxis(gc, mp->cpady);
    if (abs(ctx) < GC_STICK_DEADZONE)
        // Cpad neutral position
        is->cpadx = 0x7ff;
    else
        is->cpadx = (uint16_t) (ctx * CPAD_MAX / 0x8000 + 0x800);
    if (abs(cty) < GC_STICK_DEADZONE)
        is->cpady = 0x7ff;
    else
        is->cpady = (uint16_t) (-cty * CPAD_MAX / 0x8000 + 0x800);

    // C stick
    int16_t cx = SDL_GameControllerGetAxis(gc, mp->cstickx);
    int16_t cy = -SDL_GameControllerGetAxis(gc, mp->csticky);
    /*if (abs(cx) < GC_STICK_DEADZONE)
        // Cstick neutral position
        is->cstickx = 0x80;
    else
        is->cstickx = M_SQRT1_2 * (cx + cy) * CSTICK_MAX / 0x8000 + 0x80;*/
    /*if (abs(cy) < GC_STICK_DEADZONE)
        // Cstick neutral position
        is->csticky = 0x80;
    else
        is->csticky = M_SQRT1_2 * (cy - cx) * CSTICK_MAX / 0x8000 + 0x80;*/
    // Applying deadzone to cstick leads to unstable results. Skipping this part
    is->cstickx = (uint16_t) (M_SQRT1_2 * (cx + cy) * CSTICK_MAX / 0x8000 + 0x80);
    is->csticky = (uint16_t) (M_SQRT1_2 * (cy - cx) * CSTICK_MAX / 0x8000 + 0x80);

    // ZR, ZL
    // ZL
    if (mp->zl.m_type) {
        // Button
        is->zl = SDL_GameControllerGetButton(gc, mp->zl.m.button);
    } else {
        // Axis
        is->zl = (uint8_t) (SDL_GameControllerGetAxis(gc, mp->zl.m.axis) > TRIGGER_TO_BUTTON_DEADZONE ? 1 : 0);
    }
    // ZR
    if (mp->zr.m_type) {
        // Button
        is->zr = SDL_GameControllerGetButton(gc, mp->zr.m.button);
    } else {
        // Axis
        is->zr = (uint8_t) (SDL_GameControllerGetAxis(gc, mp->zr.m.axis) > TRIGGER_TO_BUTTON_DEADZONE ? 1 : 0);
    }


    is->home = SDL_GameControllerGetButton(gc, mp->home);
    is->power = SDL_GameControllerGetButton(gc, mp->power);

    return is;
}

int input_state_to_data(const struct _3ds_input_state *is, uint32_t data[5])
{
    // Just to be sure
    memset(data, 0, 20);

    // Notice: all O3DS normal buttons are sent inverted (0 = pressed)
    uint32_t hid = 0xfff;
    // Touch screen default
    uint32_t touch = 0x2000000;
    // Circle pad. Default: 0x7ff7ff
    uint32_t cpad;
    // C stick. Default: 0x8080
    uint16_t cstick;
    // ZR ZL
    uint16_t nhid = 0x81;
    // Home, power
    uint32_t iface = 0x0;

    // HID button state is inverted, therefore apply & to the inverse of the state
    hid &= ~(is->a << 0);
    hid &= ~(is->b << 1);
    hid &= ~(is->select << 2);
    hid &= ~(is->start << 3);
    hid &= ~(is->dpright << 4);
    hid &= ~(is->dpleft << 5);
    hid &= ~(is->dpup << 6);
    hid &= ~(is->dpdown << 7);
    hid &= ~(is->r << 8);
    hid &= ~(is->l << 9);
    hid &= ~(is->x << 10);
    hid &= ~(is->y << 11);

    cpad = (is->cpady << 12) | is->cpadx;

    cstick = (is->csticky << 8) | is->cstickx;

    nhid |= (is->zl << 10) | (is->zr << 9);

    iface |= is->home << 0;
    iface |= is->power << 1;

    // Everything together
    data[0] = hid;
    data[1] = touch;
    data[2] = cpad;
    data[3] = (cstick << 16) | nhid;
    data[4] = iface;

    return 0;
}

/*
 * Just a helper to send given input.
 */
ssize_t send_input(int sockfd, const struct sockaddr_in *addr, uint32_t data[5])
{
    return sendto(sockfd, data, 20, 0, (const struct sockaddr *) addr, sizeof(*addr));
}

/*
 * Sends current input state in one shot ignoring previous state if oldis is NULL, returning -1 on error, 1 on skipped or 0 on success.
 */
int send_gc_input(const struct gc_3ds_binding *bd, int sockfd) {
    // Current input state
    const struct _3ds_input_state *is = get_gc_input(bd->gc_controller);

    // Structure to hold data that will be sent to 3DS
    uint32_t data[5];
    // Convert to binary data
    input_state_to_data(is, data);

    // Send input to 3DS
    ssize_t res = send_input(sockfd, &bd->_3ds_addr, data);
    if (res == -1)
    {
        return -1;
    }

    // Free memory of state that's not longer in use
    free((void *) is);

    return 0;
}

int start_input_loop(struct gc_3ds_binding *bds[], const char *addrs[]) {
    // Open socket.
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sockfd == -1) {
        return -1;
    }

    // Enable processing of events
    SDL_GameControllerEventState(SDL_ENABLE);

    SDL_Event event;
    while (SDL_WaitEvent(&event)) {
        switch (event.type) {
            case SDL_CONTROLLERDEVICEADDED:
                bind_next_controller(addrs, bds, event.cdevice.which);
                break;
            case SDL_CONTROLLERDEVICEREMOVED:
                remove_controller(bds, event.cdevice.which);
                break;
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                // Whenever a button or axis changes, send new state all in a chunk,
                // so that multiple contemporary changes can be sent at once

                // Send input
                if (send_gc_input(binding_from_gc_id(event.cbutton.which, bds), sockfd) == -1) {
                    printf("Warning: Could not send data.");
                }
                break;
            case SDL_CONTROLLERAXISMOTION: {
                Uint8 a = event.caxis.axis;
                // Apply deadzones
                if ((a == SDL_CONTROLLER_AXIS_LEFTX || a == SDL_CONTROLLER_AXIS_LEFTY ||
                     a == SDL_CONTROLLER_AXIS_RIGHTX || a == SDL_CONTROLLER_AXIS_RIGHTY) &&
                    abs(event.caxis.value) < GC_STICK_DEADZONE) {
                    // If it's a stick...
                    break;
                } else if ((a == SDL_CONTROLLER_AXIS_TRIGGERLEFT || a == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) &&
                           abs(event.caxis.value) < GC_TRIGGER_DEADZONE) {
                    // ... or a trigger
                    break;
                }
                // Send input
                if (send_gc_input(binding_from_gc_id(event.caxis.which, bds), sockfd) == -1) {
                    printf("Warning: Could not send data.");
                }
                break;
            }
            case SDL_QUIT:
                printf("Interrupt detected. Exiting.");
                return 0;
            default:
                break;
        }
    }
}
