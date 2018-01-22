#ifndef REDIRECTION_H_INCLUDED
#define REDIRECTION_H_INCLUDED

#include "binding.h"
#include "mapping.h"

struct _3ds_input_state
{
    uint8_t a, b, y, x, start, home, select, power, dpup, dpleft, dpdown, dpright, zl, zr, l, r;
    uint16_t cpadx, cpady, cstickx, csticky;
};

int start_input_loop(struct gc_3ds_binding *bds[], const char *addrs[]);

#endif // REDIRECTION_H_INCLUDED
