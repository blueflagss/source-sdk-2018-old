#pragma once
#include <utils/utils.hpp>

class c_localize {
public:
    wchar_t *find( const char *token_name ) {
        return utils::get_method< wchar_t *( __thiscall * ) ( void *, const char * ) >( this, 11 )( this, token_name );
    }
};