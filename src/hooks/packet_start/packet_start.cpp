#include "packet_start.hpp"

int __fastcall hooks::packet_start::hook( REGISTERS, int incoming_sequence, int outgoing_acknowledged ) {
    globals::sent_commands.erase( std::ranges::remove_if( globals::sent_commands, [ & ]( const uint32_t &cmd ) { return abs( static_cast< int32_t >( outgoing_acknowledged - cmd ) ) >= 90; } ).begin( ), globals::sent_commands.end( ) );

    auto target_acknowledged = outgoing_acknowledged;

    for ( const auto cmd : globals::sent_commands ) {
        if ( outgoing_acknowledged >= cmd )
            target_acknowledged = cmd;
    }

    return original.fastcall< int >( ecx, edx, incoming_sequence, outgoing_acknowledged );
}

void hooks::packet_start::init( ) {
    original = safetyhook::create_inline( signature::find( "engine.dll", "56 8B F1 E8 ? ? ? ? 8B 8E ? ? ? ? 3B" ).sub( 32 ).get< void * >( ),
                                          packet_start::hook );
}
