#include "calc_view.hpp"

void __fastcall hooks::calc_view::hook( REGISTERS, vector_3d &eye_origin, vector_3d &eye_angles, float &z_near, float &z_far, float &fov ) {
    const auto entity = reinterpret_cast< c_cs_player * >( ecx );

    if ( !entity )
        return reinterpret_cast< decltype( &hooks::calc_view::hook ) >( hooks::calc_view::original.trampoline( ).address( ) )( REGISTERS_OUT, eye_origin, eye_angles, z_near, z_far, fov );

    const auto old_use_new_animation_state = entity->use_new_animstate( );

    entity->use_new_animstate( ) = false;
    reinterpret_cast< decltype( &hooks::calc_view::hook ) >( hooks::calc_view::original.trampoline( ).address( ) )( REGISTERS_OUT, eye_origin, eye_angles, z_near, z_far, fov );
    entity->use_new_animstate( ) = old_use_new_animation_state;
}

void hooks::calc_view::init( ) {
    original = safetyhook::create_inline( signature::find( "client.dll", XOR( "55 8B EC 53 8B 5D 08 56 57 FF 75 18 8B F1" ) ).get< void * >( ),
                                          calc_view::hook );
}