#include "do_post_screen_space_effects.hpp"

void __fastcall hooks::do_post_screen_space_effects::hook( REGISTERS, const c_view_setup *setup ) {
    if ( !g_vars.visuals_render_player_glow.value )
        return original.fastcall< void >( REGISTERS_OUT, setup );

    if ( !globals::local_player )
        return original.fastcall< void >( REGISTERS_OUT, setup );

    if ( !g_interfaces.engine_client->is_in_game( ) || !g_interfaces.engine_client->is_connected( ) )
        return original.fastcall< void >( REGISTERS_OUT, setup );

    for ( int i = 0; i < g_interfaces.glow_object_manager->glow_object_definitions.m_Size; i++ ) {
        auto &glow_object = g_interfaces.glow_object_manager->glow_object_definitions.Element( i );

        if ( glow_object.is_unused( ) || !glow_object.entity )
            continue;

        auto player = reinterpret_cast< c_cs_player * >( glow_object.entity );

        if ( !player->alive( ) || player->team( ) == globals::local_player->team( ) || !player->is_player( ) ) 
            continue;

        glow_object.set( static_cast< float >( g_vars.visuals_render_player_glow_color.value.r ) / 255.0f, static_cast< float >( g_vars.visuals_render_player_glow_color.value.g ) / 255.0f, static_cast< float >( g_vars.visuals_render_player_glow_color.value.b ) / 255.0f, static_cast< float >( g_vars.visuals_render_player_glow_color.value.a ) / 255.0f );
    }

    return original.fastcall< void >( REGISTERS_OUT, setup );
}

void hooks::do_post_screen_space_effects::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.client_mode, 44 ),
                                          do_post_screen_space_effects::hook );
}