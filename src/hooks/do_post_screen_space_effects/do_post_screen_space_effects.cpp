#include "do_post_screen_space_effects.hpp"
#include <features/visuals/chams.hpp>

void __fastcall hooks::do_post_screen_space_effects::hook( REGISTERS, const c_view_setup *setup ) {
    if ( !g_vars.visuals_render_player_glow.value )
        return original.fastcall< void >( REGISTERS_OUT, setup );

    if ( !globals::local_player )
        return original.fastcall< void >( REGISTERS_OUT, setup );

    if ( !g_interfaces.engine_client->is_in_game( ) || !g_interfaces.engine_client->is_connected( ) )
        return original.fastcall< void >( REGISTERS_OUT, setup );

    g_chams.on_post_screen_space_effects( );

    return original.fastcall< void >( REGISTERS_OUT, setup );
}

void hooks::do_post_screen_space_effects::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.client_mode, 44 ),
                                          do_post_screen_space_effects::hook );
}