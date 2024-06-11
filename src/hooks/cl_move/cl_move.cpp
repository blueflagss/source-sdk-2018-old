#include "cl_move.hpp"

void __cdecl hooks::cl_move::hook( float accumulated_extra_samples, bool final_tick ) {
    if ( !globals::local_player || !globals::local_player->alive( ) ) {
        original.call< void >( accumulated_extra_samples, final_tick );
        globals::sent_commands.push_back( g_interfaces.client_state->last_outgoing_command( ) );
        return;
    }

    const auto out_sequence = g_interfaces.client_state->last_outgoing_command( );

    original.call< void >( accumulated_extra_samples, final_tick );

    if ( out_sequence != g_interfaces.client_state->last_outgoing_command( ) && g_interfaces.client_state->net_channel )
        globals::sent_commands.push_back( g_interfaces.client_state->last_outgoing_command( ) );
}

void hooks::cl_move::init( ) {
    original = safetyhook::create_inline( signature::find( XOR( "engine.dll" ), XOR( "55 8B EC 81 EC ? ? ? ? 53 56 57 8B 3D ? ? ? ? 8A F9 F3 0F 11 45" ) ).get< void * >( ),
                                          cl_move::hook );
}