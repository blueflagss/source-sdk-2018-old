#include "check_jump_button.hpp"
#include <features/animations/animation_state_rebuilt.hpp>

bool hooks::check_jump_button::hook( REGISTERS ) {
    const auto ret = original.fastcall< bool >( REGISTERS_OUT );

   // if ( !globals::local_player || !globals::local_player->alive( ) )
   //     return ret;

   // auto local_animstate = globals::local_player->anim_state( );

   // if ( g_rebuilt.m_bJumping ) {
   ///*     if ( local_animstate )*/
   //         g_rebuilt.set_sequence( local_animstate, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, g_rebuilt.select_weighted_sequence( local_animstate, ACT_CSGO_JUMP ) );
   // }

    return ret;
}

void hooks::check_jump_button::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.game_movement, 38 ), 
                                          check_jump_button::hook );
}