#include "update_animation_state.hpp"
#include <features/animations/animation_sync.hpp>

void __vectorcall hooks::update_animation_state::hook( void *a1, void *a2, float a3, float a4, float a5, void *a6 ) {
    auto state = reinterpret_cast< c_csgo_player_animstate * >( a1 );

    if ( !state || !state->m_pPlayer || state->m_pPlayer == globals::local_player )
        return reinterpret_cast< decltype( &hooks::update_animation_state::hook ) >( hooks::update_animation_state::original.trampoline( ).address( ) )( a1, a2, globals::local_angles.z, globals::local_angles.y, globals::local_angles.x, a6 );
    
    return reinterpret_cast< decltype( &hooks::update_animation_state::hook ) >( hooks::update_animation_state::original.trampoline( ).address( ) )( a1, a2, a3, a4, a5, a6 );
}

void hooks::update_animation_state::init( ) {
    //original = safetyhook::create_inline( g_addresses.update_animation_state.get< void * >( ),
    //                                      update_animation_state::hook );
}