#pragma once
#include <utils/utils.hpp>

class c_model_cache {
public:
    void begin_lock( ) {
        utils::get_method< void( __thiscall * )( void * ) >( this, 32 );
    }

    void end_lock( ) {
        utils::get_method< void( __thiscall * )( void * ) >( this, 33 );
    }
};