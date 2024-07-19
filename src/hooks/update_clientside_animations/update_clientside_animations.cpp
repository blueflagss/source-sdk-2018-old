#include "update_clientside_animations.hpp"
#include <features/animations/animation_sync.hpp>

void __fastcall hooks::update_clientside_animations::hook( REGISTERS ) {
    const auto player = reinterpret_cast< c_cs_player * >( ecx );

    if ( !player )
        return original.fastcall< void >( REGISTERS_OUT );

    //if ( player == globals::local_player ) {
    //    const auto animstate = globals::local_player-> anim_state( );

    //    if ( globals::allow_animations[ globals::local_player->index( ) ] ) {
    //        /* fix viewmodel addons not updating. */
    //        animstate->m_pPlayer = nullptr;
    //        original.fastcall< void >( REGISTERS_OUT );
    //        animstate->m_pPlayer = globals::local_player;

    //        original.fastcall< void >( REGISTERS_OUT );
    //    }
    //} 
    //
    //else
    //    original.fastcall< void >( REGISTERS_OUT );

    if ( player->team( ) != globals::local_player->team( ) ) {
        player->set_abs_origin( player->origin( ) );

        if ( globals::allow_animations[ player->index( ) ] ) {
            /* @llama */
            if ( player->anim_overlays( ) ) {
                for ( int i = 0; i < ANIMATION_LAYER_COUNT; i++ )
                    player->anim_overlays( )[ i ].owner = player;
            }

            int iEFlags = player->eflags( );

            player->eflags( ) &= ~( 1 << 12 );

            original.fastcall< void >( REGISTERS_OUT );

            player->eflags( ) = iEFlags;
        }
    }
}

void hooks::update_clientside_animations::init( ) {
    //original = safetyhook::create_inline( signature::find( _xs( "client.dll" ), _xs( "55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74" ) ).get< void * >( ),
    //                                      update_clientside_animations::hook );
}