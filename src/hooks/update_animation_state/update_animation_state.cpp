#include "update_animation_state.hpp"
#include <features/animations/animation_sync.hpp>

void __vectorcall hooks::update_animation_state::hook( void *a1, void *a2, float a3, float a4, float a5, void *a6 ) {
    auto state = reinterpret_cast< c_csgo_player_animstate * >( a1 );

    if ( !state || !state->m_pPlayer )
        return reinterpret_cast< decltype( &hooks::update_animation_state::hook ) >( hooks::update_animation_state::original.trampoline( ).address( ) )( a1, a2, a3, a4, a5, a6 );
    
    if ( state->m_pPlayer == globals::local_player && !globals::allow_animations[ state->m_pPlayer->index( ) ] )
        return;
    
    return reinterpret_cast< decltype( &hooks::update_animation_state::hook ) >( hooks::update_animation_state::original.trampoline( ).address( ) )( a1, a2, a3, a4, a5, a6 );
}

void hooks::update_animation_state::init( ) {
    original = safetyhook::create_inline( g_addresses.update_animation_state.get< void * >( ),
                                          update_animation_state::hook );
}