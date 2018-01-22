#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <netinet/in.h>

#include "addrslist.h"

#define ADDRS_LIST_FILE "g3ir.list"

int read_addrs(const char *addrs[4]) {
    memset(addrs, 0, sizeof(void *) * 4);

    // Open file
    FILE *list = fopen(ADDRS_LIST_FILE, "r");
    if (!list) {
        return -1;
    }

    int c = 0;
    // Read up to four ip addresses
    while (c < 4) {
        size_t sz = INET_ADDRSTRLEN;
        // Store line
        char *ln = NULL;
        // Read lines to a maximum of 16 chars (length of an IP address + NULL terminator)
        int err = (int) getline(&ln, &sz, list);
        if (err == -1) {
            // Exit if EOF is reached
            break;
        }
        // If empty, skip
        if (!strcmp(ln, "") || !strcmp(ln, "\n")) {
            continue;
        }
        // Remove newline character if present
        strtok(ln, "\n");
        // If not empty, assign and increment c
        addrs[c] = ln;
        c++;
    }

    // Return number of addresses read
    return c;
}

void free_addrs(const char *addrs[4]) {
    for (int i = 0; i < 4; ++i) {
        free((void *) addrs[i]);
    }
}
