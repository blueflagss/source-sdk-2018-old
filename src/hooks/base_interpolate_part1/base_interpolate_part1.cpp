#include "base_interpolate_part1.hpp"

int __fastcall hooks::base_interpolate_part1::hook( REGISTERS, float &curtime, vector_3d &old_origin, vector_3d &old_angs, int &no_more_changes ) {
    const auto entity = reinterpret_cast< c_cs_player * >( ecx );

    if ( !entity )
        return reinterpret_cast< decltype( &hooks::base_interpolate_part1::hook ) >( hooks::base_interpolate_part1::original.trampoline( ).address( ) )( REGISTERS_OUT, curtime, old_origin, old_angs, no_more_changes );

    static auto move_to_last_received_pos = signature::find( XOR( "client.dll" ), XOR( "55 8B EC 80 7D 08 00 56 8B F1 75 0D 80 BE ? ? ? ? ? 0F" ) ).get< void( __thiscall * )( void *, bool ) >( );

    if ( entity->is_player( ) && entity != globals::local_player ) {
        no_more_changes = 1;
        move_to_last_received_pos( entity, false );
        return 0;
    }

    return reinterpret_cast< decltype( &hooks::base_interpolate_part1::hook ) >( hooks::base_interpolate_part1::original.trampoline( ).address( ) )( REGISTERS_OUT, curtime, old_origin, old_angs, no_more_changes );
}

void hooks::base_interpolate_part1::init( ) {
    original = safetyhook::create_inline( signature::find( XOR( "client.dll" ), XOR( "55 8B EC 51 8B 45 14 56 8B F1 C7 00 ? ? ? ? 8B 06 8B 80" ) ).get< void * >( ),
                                          base_interpolate_part1::hook );
}