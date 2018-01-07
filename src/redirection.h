#ifndef REDIRECTION_H_INCLUDED
#define REDIRECTION_H_INCLUDED

#include "binding.h"
#include "mapping.h"

struct _3ds_input_state
{
    uint8_t a, b, y, x, start, home, select, power, dpup, dpleft, dpdown, dpright, zl, zr, l, r;
    uint16_t cpadx, cpady, cstickx, csticky;
};

int send_gc_input(const struct gc_3ds_binding *bd, const struct _3ds_mapping *mp, int sockfd);

int gc_input_loop(const struct gc_3ds_binding *bd, const struct _3ds_mapping *mp, int delay_ms);

#endif // REDIRECTION_H_INCLUDED
