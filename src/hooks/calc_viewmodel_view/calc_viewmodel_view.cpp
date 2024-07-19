#include "calc_viewmodel_view.hpp"

void __fastcall hooks::calc_viewmodel_view::hook( REGISTERS, c_base_player *owner, const vector_3d &eye_position, const vector_3d &eye_angles ) {
    auto original = reinterpret_cast< decltype( &hooks::calc_viewmodel_view::hook ) >( hooks::calc_viewmodel_view::original.trampoline( ).address( ) );

    if ( !g_vars.visuals_other_viewmodel_override.value )
        return original( REGISTERS_OUT, owner, eye_position, eye_angles );

    vector_3d forward, right, up;
    math::angle_vectors( eye_angles, &forward, &right, &up );

    auto new_origin = eye_position + ( forward * g_vars.visuals_other_viewmodel_offset.value.x ) 
                                   + ( right * g_vars.visuals_other_viewmodel_offset.value.y ) 
                                   + ( up * g_vars.visuals_other_viewmodel_offset.value.z );

    return original( REGISTERS_OUT, owner, new_origin, vector_3d( eye_angles.x, eye_angles.y, eye_angles.z + g_vars.visuals_other_viewmodel_roll.value ) );
}

void hooks::calc_viewmodel_view::init( ) {
    original = safetyhook::create_inline( signature::find( "client.dll", _xs( "55 8B EC 83 EC 64 56 57" ) ).get< void * >( ),
                                          calc_viewmodel_view::hook );
}