#include <SDL2/SDL.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#include "../include/redirection.h"

#define CPAD_MAX 0x5d0
#define CSTICK_MAX 0x7f

// 60 FPS max processing time
#define MAX_PROC_TIME_NSEC 16666666.666666667

// TODO: Implement GC sticks dead zone to send "free" data to 3DS sticks.

struct input_loop_args {
    const struct gc_3ds_binding *bd;
    int delay_ms;
};

long timespec_diff_nsec(struct timespec *start, struct timespec *stop) {
    return (stop->tv_sec - start->tv_sec) * 1000000000 + stop->tv_nsec - start->tv_nsec;
}

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
        is->r = SDL_GameControllerGetAxis(gc, mp->r.m.axis) > AXIS_TO_BUTTON_DEADZONE ? 1 : 0;
    }
    // L
    if (mp->l.m_type) {
        // Button
        is->l = SDL_GameControllerGetButton(gc, mp->l.m.button);
    } else {
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
    if (mp->zl.m_type) {
        // Button
        is->zl = SDL_GameControllerGetButton(gc, mp->zl.m.button);
    } else {
        // Axis
        is->zl = SDL_GameControllerGetAxis(gc, mp->zl.m.axis) > AXIS_TO_BUTTON_DEADZONE ? 1 : 0;
    }
    // ZR
    if (mp->zr.m_type) {
        // Button
        is->zr = SDL_GameControllerGetButton(gc, mp->zr.m.button);
    } else {
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

bool input_state_equals(const struct _3ds_input_state *o, const struct _3ds_input_state *n, uint8_t cpad_e, uint8_t cstick_e)
{
    return
        o->a == n->a &&
        o->b == n->b &&
        o->select == n->select &&
        o->start == n->start &&
        o->dpright == n->dpright &&
        o->dpleft == n->dpleft &&
        o->dpup == n->dpup &&
        o->dpdown == n->dpdown &&
        o->r == n->r &&
        o->l == n->l &&
        o->x == n->x &&
        o->y == n->y &&
        abs(o->cpadx - n->cpadx) < cpad_e &&
        abs(o->cpady - n->cpady) < cpad_e &&
        abs(o->cstickx - n->cstickx) < cstick_e &&
        abs(o->csticky - n->csticky) < cstick_e &&
        o->zr == n->zr &&
        o->zl == n->zl &&
        o->home == n->home &&
        o->power == n->power;
}

/*
 * Just a helper to send given input.
 */
int send_input(int sockfd, const struct sockaddr_in *addr, uint32_t data[5])
{
    return sendto(sockfd, data, 20, 0, (const struct sockaddr *) addr, sizeof(*addr));
}

/*
 * Sends current input state in one shot ignoring previous state, returning error of sendto() or 0 on success.
 */
int send_gc_input(const struct gc_3ds_binding *bd, const struct _3ds_mapping *mp, int sockfd)
{
    const struct _3ds_input_state *is = get_gc_input(bd->gc_controller);

    uint32_t data[5];
    input_state_to_data(is, data);

    if (send_input(sockfd, &bd->_3ds_addr, data) == -1)
    {
        return -1;
    }

    free((void *) is);
    return 0;
}

void signal_handler(int signal) {
    printf("Interrupt detected. Exiting.\n");
    exit(0);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

int start_binding_input_loop(const struct gc_3ds_binding *bd, int delay_ms)
{
    // Open socket.
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sockfd == -1) {
        return -1;
    }
    // Store old state here for optimization (do not send if current state is identical)
    const struct _3ds_input_state *oldis = NULL;
    // Handle signals
    signal(SIGINT, signal_handler);

    // Monitor time and notify when delay gets too long
    struct timespec start, stop;
    long latency;
    // Main input loop
    while (1)
    {
        // Get time
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
        // Current input state
        const struct _3ds_input_state *is = get_gc_input(bd->gc_controller);
        // Skip this time if states are identical
        if (oldis != NULL && input_state_equals(oldis, is, 30, 5))
        {
            continue;
        }

        // Structure to hold data that will be sent to 3DS
        uint32_t data[5];
        // Convert to binary data
        input_state_to_data(is, data);
        // Send input to 3DS
        if (send_input(sockfd, &bd->_3ds_addr, data) == -1)
        {
            //return -1;
            printf("Warning: Unable to send input. Error msg: %s\n", strerror(errno));
        }
        // If delay is more than 0, wait
        if (delay_ms > 0)
        {
            usleep(delay_ms);
        }

        // Free memory from old state that's not longer in use
        free((void *) oldis);
        // Assign current state to old state
        oldis = is;
        // Get elapsed time
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop);
        latency = timespec_diff_nsec(&start, &stop);
        // Notify in case processing time exceeds 60 FPS
        if (latency > MAX_PROC_TIME_NSEC) {
            printf("Warning: detected high latency. Unresponsive input may occur. Latency: %lu nanoseconds\n", latency);
        }
    }
}

#pragma clang diagnostic pop

int start_routine_input_loop(struct input_loop_args *args) {
    return start_binding_input_loop(args->bd, args->delay_ms);
}

int start_input_loop(const struct gc_3ds_binding *bds[], int num_bds, int delay_ms) {
    SDL_Thread *threads[num_bds];
    // Start threads
    for (int i = 0; i < num_bds; ++i) {
        struct input_loop_args args;
        args.bd = bds[i];
        args.delay_ms = delay_ms;
        char t_name[14] = "GC3DSBinding", t_num[2];
        sprintf(t_num, "%u", i);
        strcat(t_name, t_num);
        if ((threads[i] = SDL_CreateThread(start_routine_input_loop, t_name, &args)) == NULL) {
            return -1;
        }
    }
    // Wait for threads
    for (int j = 0; j < num_bds; ++j) {
        int err = 0;
        //terr = pthread_join(threads[j], (void **) &err);
        SDL_WaitThread(threads[j], &err);
        if (err) {
            return -1;
        }
    }
    return 0;
}
