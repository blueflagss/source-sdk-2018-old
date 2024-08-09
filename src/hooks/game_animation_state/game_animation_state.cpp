#include "game_animation_state.hpp"
#include <features/animations/animation_state_rebuilt.hpp>

void __fastcall hooks::setup_velocity::hook( REGISTERS ) {
    return original.fastcall< void >( REGISTERS_OUT );
}

void __fastcall hooks::setup_alive_loop::hook( REGISTERS ) {
    return original.fastcall< void >( REGISTERS_OUT );
}

void __fastcall hooks::setup_movement::hook( REGISTERS ) {
    auto state = reinterpret_cast< c_csgo_player_animstate * >( ecx );

    if ( !state || !state->m_pPlayer || state->m_pPlayer != globals::local_player )
        return original.fastcall< void >( REGISTERS_OUT );

        return original.fastcall< void >( REGISTERS_OUT );
}

void hooks::anim_state::init( ) {
    setup_movement::original = safetyhook::create_inline( signature::find( _xs( "client.dll" ), _xs( "E8 ? ? ? ? 8B CF E8 ? ? ? ? 8B 47 60" ) ).add( 0x1 ).rel32( ).get< void * >( ),
                                                          setup_movement::hook );

    setup_alive_loop::original = safetyhook::create_inline( signature::find( _xs( "client.dll" ), _xs( "E8 ? ? ? ? 8B 47 60 83 B8" ) ).add( 0x1 ).rel32( ).get< void * >( ),
                                                            setup_alive_loop::hook );

    setup_velocity::original = safetyhook::create_inline( signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 E4 F8 83 EC 30 56 57 8B 3D" ) ).get< void * >( ),
                                                          setup_velocity::hook );
}