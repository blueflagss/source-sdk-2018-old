#include "cl_dispatch_sound.hpp"
#include "features/sound_handler/sound_handler.hpp"

void __cdecl hooks::cl_dispatch_sound::hook( const soundinfo_t &sound ) {
    if ( !globals::local_player || !globals::local_player->alive() )
        return reinterpret_cast< decltype( &hooks::cl_dispatch_sound::hook ) >( hooks::cl_dispatch_sound::original.trampoline( ).address( ) )( sound );

    if ( sound.nEntityIndex < 1 || sound.nEntityIndex > g_interfaces.global_vars->max_clients || sound.nEntityIndex == globals::local_player->index( ) )
        return reinterpret_cast< decltype( &hooks::cl_dispatch_sound::hook ) >( hooks::cl_dispatch_sound::original.trampoline( ).address( ) )( sound );

    const auto player = g_interfaces.entity_list->get_client_entity< c_cs_player * >( sound.nEntityIndex );

    if ( !player )
        return reinterpret_cast< decltype( &hooks::cl_dispatch_sound::hook ) >( hooks::cl_dispatch_sound::original.trampoline( ).address( ) )( sound );

    if ( player->dormant( ) ) {
        g_sound_handler.dormant_players[ sound.nEntityIndex ].visible = true;
        g_sound_handler.dormant_players[ sound.nEntityIndex ].last_update_time = g_interfaces.global_vars->realtime;
        g_sound_handler.dormant_players[ sound.nEntityIndex ].origin = sound.vOrigin;
    }

    return reinterpret_cast< decltype( &hooks::cl_dispatch_sound::hook ) >( hooks::cl_dispatch_sound::original.trampoline( ).address( ) )( sound );
}

void hooks::cl_dispatch_sound::init( ) {
    //original = safetyhook::create_inline( signature::find( _xs( "engine.dll" ), _xs( "E8 ? ? ? ? 8B 3D ? ? ? ? 56 E8" ) ).add( 0x1 ).rel32( ).get< void * >( ),
    //                                      cl_dispatch_sound::hook );
}