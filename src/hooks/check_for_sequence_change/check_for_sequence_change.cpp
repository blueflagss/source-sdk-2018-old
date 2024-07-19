#include "check_for_sequence_change.hpp"

void __fastcall hooks::check_for_sequence_change::hook( REGISTERS, void *hdr, int sequence, bool force_new_sequence, bool interpolate ) {
    return original.fastcall< void >( REGISTERS_OUT, hdr, sequence, force_new_sequence, 0 );
}

void hooks::check_for_sequence_change::init( ) {
    original = safetyhook::create_inline( signature::find( _xs( "client.dll" ), _xs( "55 8B EC 51 53 8B 5D 08 56 8B F1 57 85" ) ).get< void * >( ),
                                          check_for_sequence_change::hook );
}