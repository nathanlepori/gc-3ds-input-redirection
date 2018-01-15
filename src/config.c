#include <libconfig.h>
#include <string.h>
#include <stdlib.h>

#define CONFIG_FILENAME "gc-3ds-input-redirection.cfg"

int read_config_addrs(const char *addrs[]) {
    config_t cfg;
    config_init(&cfg);
    if (config_read_file(&cfg, CONFIG_FILENAME) == CONFIG_FALSE) {
        return -1;
    }

    const config_setting_t *addrs_lookup = config_lookup(&cfg, "addrs");
    int c = config_setting_length(addrs_lookup);
    for (int i = 0; i < c; i++) {
        // Allocate space for one IP address
        char *addr = malloc(16);
        strcpy(addr, config_setting_get_string_elem(addrs_lookup, i));
        addrs[i] = addr;
    }

    config_destroy(&cfg);
    return c;
}
