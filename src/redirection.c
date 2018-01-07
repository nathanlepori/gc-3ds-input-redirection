#include <SDL.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <math.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "redirection.h"
#include "mapping.h"
#include "binding.h"

#define CPAD_MAX 0x5d0
#define CSTICK_MAX 0x7f

const struct _3ds_input_state *get_gc_input(SDL_GameController *gc, const struct _3ds_mapping *mp)
{
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
    if (mp->r.m_type)
    {
        // Button
        is->r = SDL_GameControllerGetButton(gc, mp->r.m.button);
    } else
    {
        // Axis
        is->r = SDL_GameControllerGetAxis(gc, mp->r.m.axis) > AXIS_TO_BUTTON_DEADZONE ? 1 : 0;
    }
    // L
    if (mp->l.m_type)
    {
        // Button
        is->l = SDL_GameControllerGetButton(gc, mp->l.m.button);
    } else
    {
        // Axis
        is->l = SDL_GameControllerGetAxis(gc, mp->l.m.axis) > AXIS_TO_BUTTON_DEADZONE ? 1 : 0;
    }
    is->x = SDL_GameControllerGetButton(gc, mp->x);
    is->y = SDL_GameControllerGetButton(gc, mp->y);

    // Circle pad
    // 0x8000 = SDL max axis value
    is->cpadx = SDL_GameControllerGetAxis(gc, mp->cpadx) * CPAD_MAX / 0x8000 + 0x800;
    is->cpady = -SDL_GameControllerGetAxis(gc, mp->cpady) * CPAD_MAX / 0x8000 + 0x800;

    // C stick
    int16_t cx = SDL_GameControllerGetAxis(gc, mp->cstickx);
    int16_t cy = -SDL_GameControllerGetAxis(gc, mp->csticky);
    is->cstickx = M_SQRT1_2 * (cx + cy) * CSTICK_MAX / 0x8000 + 0x80;
    is->csticky = M_SQRT1_2 * (cy - cx) * CSTICK_MAX / 0x8000 + 0x80;

    // ZR, ZL
    // ZL
    if (mp->zl.m_type)
    {
        // Button
        is->zl = SDL_GameControllerGetButton(gc, mp->zl.m.button);
    } else
    {
        // Axis
        is->zl = SDL_GameControllerGetAxis(gc, mp->zl.m.axis) > AXIS_TO_BUTTON_DEADZONE ? 1 : 0;
    }
    // ZR
    if (mp->zr.m_type)
    {
        // Button
        is->zr = SDL_GameControllerGetButton(gc, mp->zr.m.button);
    } else
    {
        // Axis
        is->zr = SDL_GameControllerGetAxis(gc, mp->zr.m.axis) > AXIS_TO_BUTTON_DEADZONE ? 1 : 0;
    }


    is->home = SDL_GameControllerGetButton(gc, mp->home);

    is->power = SDL_GameControllerGetButton(gc, mp->power);

    return is;
}

int input_state_to_data(const struct _3ds_input_state *is, uint32_t data[5])
{
    memset(data, 0, 20);

    // Notice: all O3DS normal buttons are sent inverted (0 = pressed)
    uint32_t hid = 0xfff;
    // Touch screen default
    uint32_t touch = 0x2000000;
    // Circle pad
    uint32_t cpad = 0x7ff7ff;
    // C stick
    uint16_t cstick = 0x8080;
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

    //iface |= is->home << 0;
    //iface |= is->power << 1;

    // Eveything together
    data[0] = hid;
    data[1] = touch;
    data[2] = cpad;
    data[3] = (cstick << 16) | nhid;
    data[4] = iface;

    return 0;
}

int send_gc_input(const struct gc_3ds_binding *bd, const struct _3ds_mapping *mp, int sockfd)
{
    const struct _3ds_input_state *is = get_gc_input(bd->gc_controller, mp);

    uint32_t data[5];
    input_state_to_data(is, data);

    if (sendto(sockfd, data, sizeof(data), 0, (const struct sockaddr*) &bd->_3ds_addr, sizeof(bd->_3ds_addr)) == -1)
    {
        return -1;
    }

    return 0;
}

int gc_input_loop(const struct gc_3ds_binding *bd, const struct _3ds_mapping *mp, int delay_ms)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    for (;;)
    {
        send_gc_input(bd, mp, sockfd);
        if (delay_ms > 0)
        {
            usleep(delay_ms);
        }
    }
    return 0;
}
