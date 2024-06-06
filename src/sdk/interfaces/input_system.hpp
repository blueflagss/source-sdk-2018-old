#pragma once
#include <utils/utils.hpp>

class c_input_system {
public:
    void reset_input_state( ) {
        return utils::get_method< void( __thiscall * )( void * ) >( this, 25 )( this );
    }

    void enable_input( bool state ) {
        return utils::get_method< void( __thiscall * )( void *, bool ) >( this, 7 )( this, state );
    }
};