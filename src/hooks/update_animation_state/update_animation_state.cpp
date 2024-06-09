#include "update_animation_state.hpp"

void __vectorcall hooks::update_animation_state::hook( void *a1, void *a2, float a3, float a4, float a5, void *a6 ) {
    auto state = reinterpret_cast< c_csgo_player_animstate * >( a1 );

    if ( !globals::local_player || !globals::local_player->alive( ) )
        return reinterpret_cast< decltype( &hooks::update_animation_state::hook ) >( hooks::update_animation_state::original.trampoline( ).address( ) )( a1, a2, a3, a4, a5, a6 );

    if ( !state || !state->m_pPlayer || state->m_pPlayer != globals::local_player )
        return reinterpret_cast< decltype( &hooks::update_animation_state::hook ) >( hooks::update_animation_state::original.trampoline( ).address( ) )( a1, a2, a3, a4, a5, a6 );

    const auto real_angles = globals::lby_updating ? globals::sent_angles : globals::local_angles;

    return reinterpret_cast< decltype( &hooks::update_animation_state::hook ) >( hooks::update_animation_state::original.trampoline( ).address( ) )( a1, a2, real_angles.z, real_angles.y, real_angles.x, a6 );
}

void hooks::update_animation_state::init( ) {
    original = safetyhook::create_inline( g_addresses.update_animation_state.get< void * >( ),
                                          update_animation_state::hook );
}