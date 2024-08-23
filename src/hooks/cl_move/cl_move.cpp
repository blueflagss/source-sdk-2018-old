#include "cl_move.hpp"
#include <features/network_data/network_data.hpp>

void __cdecl hooks::cl_move::hook( float accumulated_extra_samples, bool final_tick ) {
    if ( !g_interfaces.client_state || !g_interfaces.engine_client->is_connected( ) || !g_interfaces.engine_client->is_in_game( ) )
        return original.call< void >( accumulated_extra_samples, final_tick );
    
    g_network_data.ping_reducer( );

    //if ( !globals::local_player || !globals::local_player->alive( ) ) {
        //globals::sent_commands.push_back( g_interfaces.client_state->last_outgoing_command( ) );
     //   return original.call< void >( accumulated_extra_samples, final_tick );
    //}

    //const auto out_sequence = g_interfaces.client_state->last_outgoing_command( );

    original.call< void >( accumulated_extra_samples, final_tick );

    //if ( out_sequence != g_interfaces.client_state->last_outgoing_command( ) && g_interfaces.client_state->net_channel )
        //globals::sent_commands.push_back( g_interfaces.client_state->last_outgoing_command( ) );
}

void hooks::cl_move::init( ) {
    original = safetyhook::create_inline( signature::find( _xs( "engine.dll" ), _xs( "55 8B EC 81 EC ? ? ? ? 53 56 57 8B 3D ? ? ? ? 8A F9 F3 0F 11 45" ) ).get< void * >( ),
                                          cl_move::hook );
}