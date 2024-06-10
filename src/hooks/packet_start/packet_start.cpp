#include "packet_start.hpp"

int __fastcall hooks::packet_start::hook( REGISTERS, int incoming_sequence, int outgoing_acknowledged ) {
    //for ( const int it : globals::sent_commands ) {
    //    if ( it == outgoing_acknowledged ) {
    //        original.fastcall< int >( ecx, edx, incoming_sequence, outgoing_acknowledged );
    //        break;
    //    }
    //}

    //for ( auto it = globals::sent_commands.begin( ); it != globals::sent_commands.end( ); ) {
    //    if ( *it < outgoing_acknowledged )
    //        it = globals::sent_commands.erase( it );
    //    else
    //        it++;
    //}

    return original.fastcall< int >( ecx, edx, incoming_sequence, outgoing_acknowledged );
}

void hooks::packet_start::init( ) {
    original = safetyhook::create_inline( signature::find( "engine.dll", "56 8B F1 E8 ? ? ? ? 8B 8E ? ? ? ? 3B" ).sub( 32 ).get< void * >( ),
                                          packet_start::hook );
}
