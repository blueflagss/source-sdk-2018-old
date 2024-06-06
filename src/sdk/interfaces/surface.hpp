#pragma once
#include <utils/utils.hpp>

class c_surface {
public:
    __inline void unlock_cursor( ) {
        return utils::get_method< void( __thiscall * )( void * ) >( this, 66 )( this );
    }

    __inline void lock_cursor( ) {
        return utils::get_method< void( __thiscall * )( void * ) >( this, 67 )( this );
    }

    __inline void set_cursor_always_visible( bool state ) {
        return utils::get_method< void( __thiscall * )( void *, bool ) >( this, 49 )( this, state );
    }
};