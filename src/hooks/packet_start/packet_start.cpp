#include "packet_start.hpp"

int __fastcall hooks::packet_start::hook( REGISTERS, int incoming_sequence, int outgoing_acknowledged ) {
    if ( !g_interfaces.client_state || !g_interfaces.engine_client->is_connected( ) || !g_interfaces.engine_client->is_in_game( ) )
        return original.fastcall< int >( REGISTERS_OUT, incoming_sequence, outgoing_acknowledged );

    if ( globals::outgoing_cmds[ outgoing_acknowledged % 150 ].cmd == outgoing_acknowledged )
        original.fastcall< int >( REGISTERS_OUT, incoming_sequence, outgoing_acknowledged );
}

void hooks::packet_start::init( ) {
    original = safetyhook::create_inline( signature::find( "engine.dll", "56 8B F1 E8 ? ? ? ? 8B 8E ? ? ? ? 3B" ).sub( 32 ).get< void * >( ),
                                          packet_start::hook );
}