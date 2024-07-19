#include "game_animation_state.hpp"
#include <features/animations/animation_state_rebuilt.hpp>

//void __fastcall hooks::setup_velocity::hook( REGISTERS ) {
//    auto state = reinterpret_cast< c_csgo_player_animstate * >( ecx );
//
//    if ( !state || !state->m_pPlayer )
//        return original.fastcall< void >( REGISTERS_OUT );
//
//    if ( !globals::allow_animations[ state->m_pPlayer->index( ) ] )
//        return original.fastcall< void >( REGISTERS_OUT );
//
//    return g_rebuilt.setup_velocity( state );
//}
//
//void __fastcall hooks::setup_alive_loop::hook( REGISTERS ) {
//    auto state = reinterpret_cast< c_csgo_player_animstate * >( ecx );
//
//    if ( !state || !state->m_pPlayer )
//        return original.fastcall< void >( REGISTERS_OUT );
//
//    if ( !globals::allow_animations[ state->m_pPlayer->index( ) ] || ( state->m_pPlayer != globals::local_player ) )
//        return original.fastcall< void >( REGISTERS_OUT );
//
//    return g_rebuilt.setup_alive_loop( state );
//}
//
//void __fastcall hooks::setup_movement::hook( REGISTERS ) {
//        auto state = reinterpret_cast< c_csgo_player_animstate * >( ecx );
//
//        if ( !state || !state->m_pPlayer )
//            return original.fastcall< void >( REGISTERS_OUT );
//
//        //if ( globals::allow_animations[ state->m_pPlayer->index( ) ] ) {
//        //const auto backup_cur_time = g_interfaces.global_vars->curtime;
//        //const auto backup_frametime = g_interfaces.global_vars->frametime;
//        //const auto backup_framecount = g_interfaces.global_vars->framecount;
//
//        //g_interfaces.global_vars->curtime = static_cast< float >( globals::local_player->tick_base() ) * g_interfaces.global_vars->interval_per_tick;
//        //g_interfaces.global_vars->frametime = g_interfaces.global_vars->interval_per_tick;
//        //g_interfaces.global_vars->framecount = globals::local_player->tick_base( );
//
//    return original.fastcall< void >( REGISTERS_OUT );
//}

void hooks::anim_state::init( ) {
  /*  setup_movement::original = safetyhook::create_inline( signature::find( XOR( "client.dll" ), XOR( "E8 ? ? ? ? 8B CF E8 ? ? ? ? 8B 47 60" ) ).add( 0x1 ).rel32( ).get< void * >( ),
                                                          setup_movement::hook );

    setup_alive_loop::original = safetyhook::create_inline( signature::find( XOR( "client.dll" ), XOR( "E8 ? ? ? ? 8B 47 60 83 B8" ) ).add( 0x1 ).rel32( ).get< void * >( ),
                                                            setup_alive_loop::hook );

    setup_velocity::original = safetyhook::create_inline( signature::find( XOR( "client.dll" ), XOR( "55 8B EC 83 E4 F8 83 EC 30 56 57 8B 3D" ) ).get< void * >( ),
                                                          setup_velocity::hook );*/
}