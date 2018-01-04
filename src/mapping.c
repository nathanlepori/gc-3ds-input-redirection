#include "mapping.h"
#include "gc_controller.h"

#define GC_MAPPING "a:b0,b:b3,y:b2,x:b1,start:b7,guide:back:,dpup:b8,dpleft:b10,dpdown:b9,dpright:b11,leftshoulder:,rightshoulder:b6,leftstick:,rightstick:,leftx:a0,lefty:a1,rightx:a3,righty:a4,lefttrigger:a2,righttrigger:a5,z:b6,platform:Linux"

// TODO: Here GUID is extracted procedurally and joined together with the other settings. If the GUID is unique for every controller it could save on processing time by hardcoding the whole mapping.
// For comparison, discovered GUID is: 0300000057696920552047616d654300

int add_gc_mapping(int port_num)
{
    // Get GUID string
    SDL_Joystick *joy = joystick_from_port(port_num);
    // Controller not connected
    if (!joy)
    {
        return -1;
    }

    SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);
    // GUID is 32 bytes long plus NULL terminator
    char guid_str[33];
    SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str));

    const char *joy_name = SDL_JoystickName(joy);

    // Allocate enough space to store GUID, name and mapping
    char m_str[32 + 1 + strlen(joy_name) + 1 + strlen(GC_MAPPING)];

    strcpy(m_str, guid_str);
    strcat(m_str, ",");
    strcat(m_str, joy_name);
    strcat(m_str, ",");
    strcat(m_str, GC_MAPPING);

    return SDL_GameControllerAddMapping(m_str);
}
