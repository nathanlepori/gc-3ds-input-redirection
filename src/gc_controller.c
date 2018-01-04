#include <regex.h>

#include <SDL.h>

#include "mapping.h"

#define BD_GC_NAME_REGEX "Wii U GameCube Adapter Port ([1-4])"

/*
Return port number (1 to 4) or 0 if the device is not a GC controller
*/
int get_gc_controller_port(int device_index)
{
    // Make space for regex and combine prefix with expression
    regex_t preg;
    // Compile regex
    if (regcomp(&preg, BD_GC_NAME_REGEX, REG_EXTENDED))
    {
        printf("Error compiling regex.");
    }

    // Get joystick name
    const char *joy_name = SDL_JoystickNameForIndex(device_index);
    // Store regex matches here (first is always full match, second is the port)
    regmatch_t pmatch[2];

    // Execute regex
    if (regexec(&preg, joy_name, 2, pmatch, 0) == REG_NOMATCH)
    {
        // Not a GC controller
        return 0;
    }

    // Port as a string
    char port_str[1];
    //Extract match
    strncpy(port_str, &joy_name[pmatch[1].rm_so], pmatch[1].rm_eo - pmatch[1].rm_so);
    // Convert to integer
    int port = atoi(port_str);

    // Free regex pointer memory
    regfree(&preg);

    //return result
    return port;
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
        int p = get_gc_controller_port(i);
        if (p != 0 && p == port_num)
        {
            return SDL_JoystickOpen(i);
        }
    }
    return NULL;
}

SDL_GameController *controller_from_port(int port_num)
{
    // Ports between 1 and 4
    if (port_num > 4 || port_num < 1)
    {
        return NULL;
    }

    // Loop through connected joysticks
    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
        // Get controller port
        int p = get_gc_controller_port(i);
        // If found...
        if (p != 0 && p == port_num)
        {
            // Add mapping
            add_gc_mapping(port_num);
            // Check if everything is OK or else return NUNLL
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
