#include "render_smoke_overlay.hpp"

void __fastcall hooks::render_smoke_overlay::hook( REGISTERS, bool pre_view_model ) {
    if ( g_vars.visuals_other_remove_scope_overlay.value )
        return;

    return original.fastcall< void >( REGISTERS_OUT, pre_view_model );
}

void hooks::render_smoke_overlay::init( ) {
    original = safetyhook::create_inline( signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 E4 F0 83 EC 48 80 7D 08 00" ) ).get< void * >( ),
                                          render_smoke_overlay::hook );
}