#include "calc_viewmodel_bob.hpp"

float __fastcall hooks::calc_viewmodel_bob::hook( REGISTERS ) {
    if ( g_vars.visuals_other_remove_view_bob.value )
        return 0.0f;

    return original.fastcall< float >( REGISTERS_OUT );
}

void hooks::calc_viewmodel_bob::init( ) {
    original = safetyhook::create_inline( signature::find( "client.dll", _xs( "55 8B EC A1 ? ? ? ? 83 EC 10 8B 40 34" ) ).get< void * >( ),
                                          calc_viewmodel_bob::hook );
}