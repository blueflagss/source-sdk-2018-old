#include "modify_eye_position.hpp"

void __fastcall hooks::modify_eye_position::hook( REGISTERS, vector_3d &eye_position ) {
    const auto anim_state = reinterpret_cast< c_csgo_player_animstate * >( ecx );

    if ( !anim_state || !anim_state->m_pPlayer || anim_state->m_pPlayer != globals::local_player )
        return reinterpret_cast< decltype( &hooks::modify_eye_position::hook ) >( hooks::modify_eye_position::original.trampoline( ).address( ) )( REGISTERS_OUT, eye_position );

	anim_state->m_bSmoothHeightValid = false;

    return reinterpret_cast< decltype( &hooks::modify_eye_position::hook ) >( hooks::modify_eye_position::original.trampoline( ).address( ) )( REGISTERS_OUT, eye_position );
}

void hooks::modify_eye_position::init( ) {
    original = safetyhook::create_inline( signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 E4 F8 83 EC 58 56 57 8B F9 83 7F 60" ) ).get< void * >( ),
                                          modify_eye_position::hook );
}