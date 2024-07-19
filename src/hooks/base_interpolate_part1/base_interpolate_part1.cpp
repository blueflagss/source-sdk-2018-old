#include "base_interpolate_part1.hpp"
#include <features/animations/animation_sync.hpp>

int __fastcall hooks::base_interpolate_part1::hook( REGISTERS, float &curtime, vector_3d &old_origin, vector_3d &old_angs, int &no_more_changes ) {
    return reinterpret_cast< decltype( &hooks::base_interpolate_part1::hook ) >( hooks::base_interpolate_part1::original.trampoline( ).address( ) )( REGISTERS_OUT, curtime, old_origin, old_angs, no_more_changes );
}

void hooks::base_interpolate_part1::init( ) {
    original = safetyhook::create_inline( signature::find( _xs( "client.dll" ), _xs( "55 8B EC 51 8B 45 14 56 8B F1 C7 00 ? ? ? ? 8B 06 8B 80" ) ).get< void * >( ),
                                          base_interpolate_part1::hook );
}