#include "estimate_abs_velocity.hpp"

void __fastcall hooks::estimate_abs_velocity::hook( REGISTERS, vector_3d &velocity ) {
    auto entity = reinterpret_cast< c_cs_player * >( ecx );

    if ( !entity )
        return reinterpret_cast< decltype( &hooks::estimate_abs_velocity::hook ) >( hooks::estimate_abs_velocity::original.trampoline( ).address( ) )( REGISTERS_OUT, velocity );

    if ( entity->is_player( ) ) {
        velocity = entity->velocity( );
        return;
    }

    return reinterpret_cast< decltype( &hooks::estimate_abs_velocity::hook ) >( hooks::estimate_abs_velocity::original.trampoline( ).address( ) )( REGISTERS_OUT, velocity );
}

void hooks::estimate_abs_velocity::init( ) {
    original = safetyhook::create_inline( signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 E4 F8 83 EC 0C 56 8B F1 85 F6" ) ).get< void * >( ),
                                          estimate_abs_velocity::hook );
}