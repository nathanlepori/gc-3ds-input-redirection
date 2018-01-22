#include <regex.h>

#include <SDL2/SDL.h>
#include <stdbool.h>

#define BD_GC_NAME_REGEX "Wii U GameCube Adapter Port ([1-4])"

int controller_port_from_name(const char *cname) {
    regex_t preg;
    // Compile regex
    if (regcomp(&preg, BD_GC_NAME_REGEX, REG_EXTENDED)) {
        printf("Error compiling regex.");
    }

    // Store regex matches here (first is always full match, second is the port)
    regmatch_t pmatch[2];

    // Execute regex
    if (regexec(&preg, cname, 2, pmatch, 0) == REG_NOMATCH) {
        // Not a GC controller
        return -1;
    }

    // Port as a string
    char port_str[1];
    //Extract match
    strncpy(port_str, &cname[pmatch[1].rm_so], (size_t) (pmatch[1].rm_eo - pmatch[1].rm_so));
    // Convert to integer and subtract 1 (port range is 0 to 3 to match array indices)
    int port = (int) strtol(port_str, NULL, 10) - 1;

    // Free regex pointer memory
    regfree(&preg);

    //return result
    return port;
}

/*
Return port number (0 to 3) or -1 if the device is not a GC controller
*/
int controller_port_from_index(int device_index) {
    // Get controller name
    const char *cname = SDL_JoystickNameForIndex(device_index);

    // Get port
    return controller_port_from_name(cname);
}

int controller_port_from_id(int inst_id) {
    SDL_Joystick *joy = SDL_JoystickFromInstanceID(inst_id);
    const char *cname = SDL_JoystickName(joy);

    return controller_port_from_name(cname);
}

bool is_gc_controller(int device_index) {
    return controller_port_from_index(device_index) != -1;
}

SDL_Joystick *joystick_from_port(int port_num)
{
    // Ports between 1 and 4
    if (port_num > 4 || port_num < 1)
    {
        return NULL;
    }

    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
        int p = controller_port_from_index(i);
        if (p != -1 && p == port_num)
        {
            return SDL_JoystickOpen(i);
        }
    }
    return NULL;
}

SDL_GameController *controller_from_port(int port_num)
{
    // Ports between 0 and 3
    if (port_num > 3)
    {
        return NULL;
    }

    // Loop through connected joysticks
    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
        // Get controller port
        int p = controller_port_from_index(i);
        // If found...
        if (p != -1 && p == port_num)
        {
            // Check if everything is OK or else return NULL
            if (!SDL_IsGameController(i))
            {
                return NULL;
            }
            // Return controller
            return SDL_GameControllerOpen(i);
        }
    }
    return NULL;
}


