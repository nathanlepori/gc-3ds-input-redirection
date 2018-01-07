#include "mapping.h"
#include "gc_controller.h"

#define GC_MAPPING "a:b0,b:b3,y:b2,x:b1,start:b7,guide:back:,dpup:b8,dpleft:b10,dpdown:b9,dpright:b11,leftshoulder:,rightshoulder:b6,leftstick:,rightstick:,leftx:a0,lefty:a1,rightx:a3,righty:a4,lefttrigger:a2,righttrigger:a5,z:b6,platform:Linux"

const struct _3ds_mapping default_3ds_mapping = (struct _3ds_mapping) {
    .a = SDL_CONTROLLER_BUTTON_A,
    .b = SDL_CONTROLLER_BUTTON_B,
    .y = SDL_CONTROLLER_BUTTON_Y,
    .x = SDL_CONTROLLER_BUTTON_X,

    .start = SDL_CONTROLLER_BUTTON_START,
    .home = SDL_CONTROLLER_BUTTON_GUIDE,
    .select = SDL_CONTROLLER_BUTTON_BACK,
    .power = SDL_CONTROLLER_BUTTON_INVALID,

    .dpup = SDL_CONTROLLER_BUTTON_DPAD_UP,
    .dpleft = SDL_CONTROLLER_BUTTON_DPAD_LEFT,
    .dpdown = SDL_CONTROLLER_BUTTON_DPAD_DOWN,
    .dpright = SDL_CONTROLLER_BUTTON_DPAD_RIGHT,

    .zl.m_type = 1,
    .zl.m.button = SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
    .zr.m_type = 1,
    .zr.m.button = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
    .l.m_type = 0,
    .l.m.axis = SDL_CONTROLLER_AXIS_TRIGGERLEFT,
    .r.m_type = 0,
    .r.m.axis = SDL_CONTROLLER_AXIS_TRIGGERRIGHT,

    .cpadx = SDL_CONTROLLER_AXIS_LEFTX,
    .cpady = SDL_CONTROLLER_AXIS_LEFTY,
    .cstickx = SDL_CONTROLLER_AXIS_RIGHTX,
    .csticky = SDL_CONTROLLER_AXIS_RIGHTY
};

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
