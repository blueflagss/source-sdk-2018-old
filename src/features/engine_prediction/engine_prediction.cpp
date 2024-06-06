#include "engine_prediction.hpp"

void engine_prediction::post_think( c_base_entity *player ) {
    g_interfaces.model_cache->begin_lock( );

    static auto post_think_vphysics = signature::find( "client.dll", XOR( "55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 8B D9 56 57 83 BB" ) ).get< bool( __thiscall * )( c_base_entity * ) >( );
    static auto simulate_player_simulated_entities = signature::find( "client.dll", XOR( "56 8B F1 57 8B BE ? ? ? ? 83 EF 01 78 72 90 8B 86" ) ).get< void( __thiscall * )( void * ) >( );

    if ( player->alive( ) ) {
        utils::get_method< void( __thiscall * )( void * ) >( player, 329 )( player );

        if ( player->flags( ) & player_flags::on_ground )
            player->fall_velocity( ) = 0.f;

        if ( player->sequence( ) == -1 )
            player->set_sequence( 0 );

        utils::get_method< void( __thiscall * )( void * ) >( player, 214 )( player );
        post_think_vphysics( player );
    }

    simulate_player_simulated_entities( player );
    g_interfaces.model_cache->end_lock( );
}

void engine_prediction::start( c_user_cmd *ucmd ) {
    if ( !globals::local_player || !ucmd || !g_interfaces.move_helper )
        return;

    if ( g_interfaces.client_state->delta_tick > 0 ) {
        g_interfaces.prediction->update(
                g_interfaces.client_state->delta_tick,
                g_interfaces.client_state->delta_tick > 0,
                g_interfaces.client_state->last_command_ack,
                g_interfaces.client_state->last_outgoing_command + g_interfaces.client_state->choked_commands
        );
    }

    globals::local_player->current_cmd( ) = ucmd;
    globals::local_player->last_cmd( ) = *ucmd;

    if ( !prediction_player || !prediction_seed ) {
        prediction_seed = signature::find( "client.dll", XOR( "8B 47 40 A3" ) ).add( 4 ).deref( ).get< std::uintptr_t >( );
        prediction_player = signature::find( "client.dll", XOR( "0F 5B C0 89 35" ) ).add( 5 ).deref( ).get< std::uintptr_t >( );
    }

    *reinterpret_cast< int * >( prediction_seed ) = ucmd ? ucmd->random_seed : -1;
    *reinterpret_cast< c_cs_player ** >( prediction_player ) = globals::local_player;

    backup_curtime = g_interfaces.global_vars->curtime;
    backup_frametime = g_interfaces.global_vars->frametime;

    g_interfaces.global_vars->curtime = globals::local_player->tick_base( ) * g_interfaces.global_vars->interval_per_tick;

    backup_first_time_predicted = g_interfaces.prediction->first_time_predicted;
    backup_in_prediction = g_interfaces.prediction->in_prediction;

    g_interfaces.prediction->first_time_predicted = false;
    g_interfaces.prediction->in_prediction = true;

    ucmd->buttons |= globals::local_player->button_forced( );
    //	ucmd->m_buttons &= ~globals::local_player->button_disabled ( );

    g_interfaces.move_helper->set_host( globals::local_player );
    g_interfaces.game_movement->start_track_prediction_errors( globals::local_player );

    predicted_velocity = globals::local_player->velocity( );
    predicted_flags = globals::local_player->flags( );

    if ( ucmd->weapon_select != 0 && globals::local_weapon ) {
        auto data = globals::local_weapon->get_weapon_data( );

        if ( data )
            globals::local_player->select_item( data->weapon_name, ucmd->weapon_subtype );
    }

    const auto buttons_changed = ucmd->buttons ^ globals::local_player->button_last( );

    globals::local_player->buttons( ) = globals::local_player->button_last( );
    globals::local_player->button_last( ) = ucmd->buttons;
    globals::local_player->button_pressed( ) = ucmd->buttons & buttons_changed;
    globals::local_player->button_released( ) = buttons_changed & ~ucmd->buttons;

    g_interfaces.prediction->check_moving_ground( globals::local_player, g_interfaces.global_vars->frametime );
    g_interfaces.prediction->set_local_view_angles( ucmd->view_angles );

    if ( globals::local_player->physics_run_think( 0 ) )
        globals::local_player->pre_think( );

    const auto think_tick = globals::local_player->think_tick( );

    if ( think_tick > 0 && think_tick <= globals::local_player->tick_base( ) ) {
        globals::local_player->think_tick( ) = -1;
        static auto set_next_think = signature::find( "client.dll", XOR( "55 8B EC 56 57 8B F9 8B B7 ? ? ? ? 8B C6 C1 E8 16 24 01 74 18" ) ).get< void( __thiscall * )( void *, int ) >( );
        set_next_think( globals::local_player, 0 );
        globals::local_player->think( );
    }

    g_interfaces.prediction->setup_move( globals::local_player, ucmd, g_interfaces.move_helper, &move_data );
    g_interfaces.game_movement->process_movement( globals::local_player, &move_data );

    g_interfaces.prediction->finish_move( globals::local_player, ucmd, &move_data );
    g_interfaces.move_helper->process_impacts( );

    post_think( globals::local_player );
}

void engine_prediction::finish( c_user_cmd *ucmd ) {
    if ( !globals::local_player || !ucmd || !g_interfaces.move_helper )
        return;

    g_interfaces.game_movement->finish_track_prediction_errors( globals::local_player );
    g_interfaces.move_helper->set_host( nullptr );

    if ( globals::local_weapon ) {
        auto weapon_data = globals::local_weapon->get_weapon_data( );

        if ( weapon_data ) {
            if ( !globals::local_weapon->is_grenade( ) )
                globals::local_weapon->update_accuracy_penalty( );

            auto weapon_id = globals::local_weapon->item_definition_index( );

            auto is_special_weapon = weapon_id == 9 || weapon_id == 11 || weapon_id == 38 || weapon_id == 40;

            ideal_inaccuracy = 0.0f;

            if ( weapon_id == weapons::ssg08 && !( globals::local_player->flags( ) & player_flags::on_ground ) )
                ideal_inaccuracy = 0.00875f;

            else if ( globals::local_player->flags( ) & player_flags::ducking ) {
                auto weapon_data = globals::local_weapon->get_weapon_data( );

                if ( weapon_data ) {
                    if ( is_special_weapon )
                        ideal_inaccuracy = weapon_data->inaccuracy_crouch_alt;
                    else
                        ideal_inaccuracy = weapon_data->inaccuracy_crouch;
                }
            }

            else if ( is_special_weapon )
                ideal_inaccuracy = weapon_data->inaccuracy_stand_alt;
            else
                ideal_inaccuracy = weapon_data->inaccuracy_stand;
        }
    }

    globals::local_player->current_cmd( ) = nullptr;

    *reinterpret_cast< int * >( prediction_seed ) = -1;
    *reinterpret_cast< c_cs_player ** >( prediction_player ) = nullptr;

    g_interfaces.game_movement->reset( );

    g_interfaces.global_vars->curtime = backup_curtime;
    g_interfaces.global_vars->frametime = backup_frametime;

    g_interfaces.prediction->first_time_predicted = backup_first_time_predicted;
    g_interfaces.prediction->in_prediction = backup_in_prediction;
}

prediction_data &engine_prediction::get_prediction_info( const int current_command ) {
    return pred_data[ current_command % 90 ];
}