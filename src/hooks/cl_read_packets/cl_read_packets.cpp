#include "cl_read_packets.hpp"

void __fastcall hooks::cl_read_packets::hook( bool a1 ) {
    bool read = false;

    if ( !g_interfaces.engine_client->is_in_game( ) )
        read = true;

    if ( auto nci = g_interfaces.engine_client->get_net_channel_info( ) ) {
        if ( nci->is_loopback( ) )
            read = true;
    }

    if ( read || !globals::backup_global_vars.tick_count ) {
        original.fastcall< void >( a1 );
    } else if ( g_interfaces.client_state ) {
        const auto cl = ( uintptr_t ) g_interfaces.client_state + 8;

        *( int * ) ( cl + 0x164 ) = globals::backup_clientstate_vars[ 0 ];
        *( int * ) ( cl + 0x168 ) = globals::backup_clientstate_vars[ 1 ];
        *( int * ) ( cl + 0x4C98 ) = globals::backup_clientstate_vars[ 2 ];

        g_interfaces.global_vars->restore( globals::backup_global_vars );
    }

    //original.fastcall< void >( a1 );
}

void hooks::cl_read_packets::init( ) {
    original = safetyhook::create_inline( signature::find( _xs( "engine.dll" ), _xs( "53 8A D9 8B 0D ? ? ? ? 56 57 8B B9" ) ).get< void * >( ),
                                          cl_read_packets::hook );
}