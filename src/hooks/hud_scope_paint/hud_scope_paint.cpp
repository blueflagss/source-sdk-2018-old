#include "hud_scope_paint.hpp"

void __fastcall hooks::hud_scope_paint::hook( REGISTERS ) {
    if ( g_vars.visuals_other_remove_scope_overlay.value )
        return;

    return original.fastcall< void >( REGISTERS_OUT );
}

void hooks::hud_scope_paint::init( ) {
    original = safetyhook::create_inline( signature::find( XOR( "client.dll" ), XOR( "55 8B EC 83 E4 F8 83 EC 78 56 57 8B 3D" ) ).get< void * >( ),
                                          hud_scope_paint::hook );
}