#pragma once
#include <utils/utils.hpp>

class c_panel {
public:
    __inline void set_keyboard_input_enabled( unsigned int panel, bool state ) {
        return utils::get_method< void( __thiscall * )( void *, unsigned int, bool ) >( this, 31 )( this, panel, state );
    }

    __inline void set_mouse_input_enabled( unsigned int panel, bool state ) {
        return utils::get_method< void( __thiscall * )( void *, unsigned int, bool ) >( this, 32 )( this, panel, state );
    }

    __inline const char *get_name( unsigned int panel ) {
        return utils::get_method< const char *( __thiscall * ) ( void *, unsigned int ) >( this, 36 )( this, panel );
    }
};