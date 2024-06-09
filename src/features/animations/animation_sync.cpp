#include "animation_sync.hpp"
#include <features/movement/movement.hpp>

void lag_record::reset( c_cs_player *player ) {
    this->player = player;

    flags = player->flags( );
    origin = player->origin( );
    mins = player->mins( );
    maxs = player->maxs( );
    sim_time = player->simtime( );
    old_sim_time = player->old_simtime( );
    abs_angles = player->get_abs_angles( );
    abs_origin = player->get_abs_origin( );
    velocity = player->velocity( );
    anim_state = player->anim_state( );
    mode = resolve_mode::none;
    velocity = player->velocity( );
    index = player->index( );
    eye_angles = player->eye_angles( );
    model = player->get_model( );
    anim_time = player->old_simtime( ) + g_interfaces.global_vars->interval_per_tick;
    choked = std::clamp< int >( game::time_to_ticks( player->simtime( ) - old_sim_time ), 0, 16 );

    if ( player->bone_cache( ) )
        memcpy( this->bones.data( ), player->bone_cache( ), player->bone_count( ) * sizeof( matrix_3x4 ) );

    memcpy( this->pose_parameters.data( ), player->pose_parameters( ).data( ), sizeof( float ) * 24 );

    record_filled = true;
}

bool lag_record::is_valid( ) {
    auto net_channel = g_interfaces.engine_client->get_net_channel_info( );

    if ( !net_channel )
        return false;

    const float curtime = globals::local_player->alive( ) ? game::ticks_to_time( globals::local_player->tick_base( ) ) : g_interfaces.global_vars->curtime;

    float correct = 0.0f;

    correct += net_channel->get_latency( 1 );
    correct += net_channel->get_latency( 0 );
    correct += globals::lerp_amount;

    correct = std::clamp< float >( correct, 0.0f, globals::cvars::sv_maxunlag->get_float( ) );

    const float dead_time = curtime - globals::cvars::sv_maxunlag->get_float( );

    if ( this->sim_time <= dead_time )
        return false;

    return std::fabs( correct - ( curtime - this->sim_time ) ) <= 0.19f;
}

void lag_record::cache( ) {
    if ( !this || !player )
        return;

    auto cache = player->bone_cache( );

    if ( !cache )
        return;

    player->origin( ) = this->origin;
    player->set_collision_bounds( this->mins, this->maxs );
    player->set_abs_angles( this->abs_angles );
    player->invalidate_bone_cache( );
    player->bone_cache( ) = this->bones.data( );
}

bool animation_sync::get_lagcomp_bones( c_cs_player *player, std::array< matrix_3x4, 128 > &out ) {
    auto net_channel = g_interfaces.engine_client->get_net_channel_info( );

    if ( !net_channel )
        return false;

    auto time_valid_no_deadtime = [ & ]( float t ) {
        const auto correct = std::clamp< float >( net_channel->get_latency( 1 ) + net_channel->get_latency( 0 ) + globals::lerp_amount, 0.0f, globals::cvars::sv_maxunlag->get_float( ) );

        return fabs( correct - ( g_interfaces.global_vars->curtime - t ) ) <= 0.2f;
    };

    auto &log = g_animations.lag_info[ player->index( ) ];

    if ( !log.anim_records.empty( ) ) {
        const auto lag_records = log.anim_records;

        for ( int i = static_cast< int >( lag_records.size( ) ) - 1; i >= 0; i-- ) {
            auto &record = lag_records[ i ];

            if ( time_valid_no_deadtime( lag_records[ i ].sim_time ) ) {
                if ( glm::length( record.origin - player->origin( ) ) < 1.0f )
                    return false;

                bool end = ( i - 1 ) <= 0;
                vector_3d next = end ? player->origin( ) : lag_records[ i - 1 ].origin;
                float time_next = end ? player->simtime( ) : lag_records[ i - 1 ].sim_time;

                float correct = net_channel->get_latency( 0 ) + net_channel->get_latency( 1 ) + globals::lerp_amount;
                float time_delta = time_next - lag_records[ i ].sim_time;
                float add = end ? 0.2f : time_delta;
                float deadtime = lag_records[ i ].sim_time;

                float curtime = g_interfaces.global_vars->curtime;
                float delta = deadtime - curtime;

                float mul = 1.f / add;

                auto lerp = next + ( lag_records[ i ].origin - next ) * std::clamp( delta * mul, 0.0f, 1.0f );

                out = lag_records[ i ].bones;

                for ( auto &iter : out )
                    iter.set_origin( iter.get_origin( ) - lag_records[ i ].origin + lerp );

                return true;
            }
        }
    }

    return false;
};

bool animation_sync::build_bones( c_cs_player *player, matrix_3x4 *out, float curtime ) {
    if ( !player->index( ) || player->index( ) > 64 )
        return false;

    g_interfaces.model_cache->begin_lock( );

    const auto cur_time = g_interfaces.global_vars->curtime;
    const auto frame_time = g_interfaces.global_vars->frametime;
    const auto abs_frame_time = g_interfaces.global_vars->abs_frametime;
    const auto interval_per_tick = curtime / ( g_interfaces.global_vars->interval_per_tick + 0.5f );
    const auto frame_count = g_interfaces.global_vars->framecount;
    const auto tick_count = g_interfaces.global_vars->tick_count;

    g_interfaces.global_vars->curtime = curtime;
    g_interfaces.global_vars->frametime = g_interfaces.global_vars->interval_per_tick;
    g_interfaces.global_vars->abs_frametime = g_interfaces.global_vars->interval_per_tick;
    g_interfaces.global_vars->framecount = interval_per_tick;
    g_interfaces.global_vars->tick_count = interval_per_tick;

    const auto backup_effects = player->effects( );
    const auto backup_setup_time = player->last_bone_setup_time( );
    const auto backup_maintain_sequence_transitions = player->maintain_sequence_transitions( );

    player->maintain_sequence_transitions( ) = false;
    player->last_bone_setup_time( ) = 0;
    player->effects( ) |= effects::nointerp;

    globals::is_building_bones[ player->index( ) ] = true;
    bool setup = player->setup_bones( out, 128, 0x7FF00, curtime );
    globals::is_building_bones[ player->index( ) ] = false;

    player->maintain_sequence_transitions( ) = backup_maintain_sequence_transitions;
    player->last_bone_setup_time( ) = backup_setup_time;
    player->effects( ) &= ~effects::nointerp;

    g_interfaces.global_vars->curtime = cur_time;
    g_interfaces.global_vars->frametime = frame_time;
    g_interfaces.global_vars->abs_frametime = abs_frame_time;
    g_interfaces.global_vars->framecount = frame_count;
    g_interfaces.global_vars->tick_count = tick_count;

    g_interfaces.model_cache->end_lock( );

    return setup;
}

void animation_sync::update_land( c_cs_player *player, lag_record *record, lag_record *last_record ) {
    if ( !last_record )
        return;

    bool on_ground = player->flags( ) & player_flags::on_ground;

    record->on_ground = false;
    record->real_on_ground = on_ground;

    if ( on_ground && last_record->real_on_ground ) {
        record->on_ground = true;
    } else {
        if ( record->layer_records[ ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ].weight != 1.f && record->layer_records[ ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ].weight == 1.f && record->layer_records[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ].weight != 0.f ) {
            record->on_ground = true;
        }

        if ( on_ground ) {
            bool ground = record->on_ground;
            if ( !last_record->real_on_ground )
                ground = false;

            record->on_ground = ground;
        }
    }
}

void animation_sync::update_velocity( c_cs_player *player, lag_record *record, lag_record *previous ) {
    if ( !previous )
        return;

    auto weapon = g_interfaces.entity_list->get_client_entity_from_handle< c_cs_weapon_base * >( player->weapon_handle( ) );

    if ( !weapon )
        return;

    player_info_t player_info;
    g_interfaces.engine_client->get_player_info( player->index( ), &player_info );

    if ( !player_info.fake_player ) {
        if ( record->choked > 0 && record->choked < 16 && previous && !previous->dormant )
            record->velocity = ( record->origin - previous->origin ) * ( 1.f / game::ticks_to_time( record->choked ) );

        record->anim_velocity = record->velocity;

        if ( record->flags & player_flags::on_ground && record->velocity.length( ) > 0.1f && record->layer_records[ ANIMATION_LAYER_LEAN ].weight == 0.f && record->layer_records[ ANIMATION_LAYER_MOVEMENT_MOVE ].weight < 0.1f )
            record->fake_walk = true;

        if ( player->flags( ) & player_flags::on_ground && record->layer_records[ ANIMATION_LAYER_ALIVELOOP ].weight > 0.f && record->layer_records[ ANIMATION_LAYER_ALIVELOOP ].weight < 1.f && record->layer_records[ ANIMATION_LAYER_ALIVELOOP ].cycle > previous->layer_records[ ANIMATION_LAYER_ALIVELOOP ].cycle ) {
            if ( weapon ) {
                float max_speed = 260.f;
                auto weapon_info = weapon->get_weapon_data( );

                if ( weapon_info )
                    max_speed = player->scoped( ) ? weapon_info->max_speed_alt : weapon_info->max_speed;

                float modifier = 0.35f * ( 1.f - record->layer_records[ ANIMATION_LAYER_ALIVELOOP ].weight );

                if ( modifier > 0.f && modifier < 1.f )
                    record->anim_speed = max_speed * ( modifier + 0.55f );
            }
        }

        if ( record->anim_speed > 0.f ) {
            record->anim_speed /= glm::length( record->velocity );

            record->anim_velocity *= record->anim_speed;
        }

        if ( record->fake_walk )
            record->anim_velocity = { 0.f, 0.f, 0.f };
    }
}

void animation_sync::update_local_animations( c_user_cmd *user_cmd ) {
    if ( !globals::local_player )
        return;

    if ( g_interfaces.client_state->choked_commands( ) != 0 )
        return;

    auto state = globals::local_player->anim_state( );

    if ( !state ) {
        return;
    }

    const auto backup_angle = globals::local_player->pl( ).v_angle;

    last_angle = user_cmd->view_angles.y;

    glm::vec3 angles = user_cmd->view_angles;

    if ( state->m_bLanding ) {
        angles.x = -10.f;
    }

    globals::local_player->pl( ).v_angle = angles;
    globals::local_player->flags( ) &= ~player_flags::on_ground;

    if ( g_movement.ground_ticks >= 2 ) {
        globals::local_player->flags( ) |= player_flags::on_ground;
    }

    memcpy( animation_layers.data( ), globals::local_player->anim_overlays( ), sizeof( c_animation_layer ) * 13 );
    pose_parameters = globals::local_player->pose_parameters( );

    const auto client_side_backup = globals::local_player->client_side_animation( );

    globals::allow_animations[ globals::local_player->index( ) ] = globals::local_player->client_side_animation( ) = true;
    g_rebuilt.update_animation_state( state, globals::local_angles, globals::local_player->simtime( ) );
    globals::allow_animations[ globals::local_player->index( ) ] = false;
    globals::local_player->client_side_animation( ) = client_side_backup;

    animated_origin[ globals::local_player->index( ) ] = globals::local_player->origin( ); 
    build_bones( globals::local_player, animated_bones[ globals::local_player->index( ) ].data( ), globals::local_player->simtime( ) );
    globals::local_player->attachment_helper( );

    if ( g_movement.ground_ticks ) {
        globals::local_player->flags( ) |= player_flags::on_ground;
    }

    if ( state->m_bOnGround ) {
        if ( state->m_flVelocityLengthXY > 0.1f ) {
            lower_body_realign_timer = g_interfaces.global_vars->curtime + 0.22f;
        }

        else if ( g_interfaces.global_vars->curtime > lower_body_realign_timer && std::abs( valve_math::angle_diff( state->m_flFootYaw, state->m_flEyeYaw ) ) > 35.f ) {
            lower_body_realign_timer = g_interfaces.global_vars->curtime + 1.1f;
        }
    }

    memcpy( globals::local_player->anim_overlays( ), animation_layers.data( ), sizeof( c_animation_layer ) * 13 );
    globals::local_player->pose_parameters( ) = pose_parameters;

    foot_yaw = state->m_flFootYaw;
    globals::local_player->pl( ).v_angle = backup_angle;
}

void animation_sync::maintain_local_animations( ) {
    if ( !globals::local_player )
        return;

    float simulation_time = globals::local_player->simtime( );

    if ( !g_interfaces.client_state->choked_commands( ) )
        memcpy( animation_layers.data( ), globals::local_player->anim_overlays( ), sizeof( c_animation_layer ) * 13 );

    globals::local_player->set_abs_angles( glm::vec3{ 0.f, foot_yaw, 0.f } );
    memcpy( globals::local_player->anim_overlays( ), animation_layers.data( ), sizeof( c_animation_layer ) * 13 );
    globals::local_player->pose_parameters( ) = pose_parameters;
}

void animation_sync::update_player_animation( c_cs_player *player, lag_record &record, lag_record *previous ) {
    record.reset( player );

    const auto state = player->anim_state( );

    if ( !state )
        return;

    record.pose_parameters = player->pose_parameters( );

    std::memcpy( record.layer_records.data( ), player->anim_overlays( ), record.layer_records.size( ) );
    std::memcpy( record.anim_state, state, sizeof( record.anim_state ) );

    record.break_lc = false;

    if ( previous ) {
        if ( record.sim_time > record.old_sim_time ) {
            if ( math::length_2d( record.origin - previous->origin ) > 4096.f )// LAG_COMPENSATION_TELEPORTED_DISTANCE_SQR = 64.f * 64.f = 4096.f
                record.break_lc = true;
        }
    }

    if ( record.sim_time <= record.old_sim_time )
        record.break_lc = true;

    if ( record.break_lc )
        clear_data( player->index( ) );

    auto backup_velocity = player->velocity( );
    auto backup_abs_velocity = player->abs_velocity( );
    auto backup_abs_origin = player->get_abs_origin( );
    auto backup_origin = player->origin( );
    auto backup_flags = player->flags( );
    auto backup_eflags = player->eflags( );

    auto update_client_side_animations = [ & ]( c_cs_player *player ) -> void {
        auto last_client_side_anim = player->client_side_animation( );

        globals::allow_animations[ player->index( ) ] = player->client_side_animation( ) = true;
        g_rebuilt.update_animation_state( state, player->eye_angles( ), record.anim_time );
        globals::allow_animations[ player->index( ) ] = false;
        player->client_side_animation( ) = last_client_side_anim;
    };

    c_global_vars_base backup_vars{ };
    backup_vars.backup( g_interfaces.global_vars );

    g_interfaces.global_vars->curtime = record.anim_time;
    g_interfaces.global_vars->frametime = g_interfaces.global_vars->interval_per_tick;
    g_interfaces.global_vars->realtime = record.anim_time;
    g_interfaces.global_vars->abs_frametime = g_interfaces.global_vars->interval_per_tick;
    g_interfaces.global_vars->framecount = game::time_to_ticks( record.anim_time );
    g_interfaces.global_vars->tick_count = game::time_to_ticks( record.anim_time );
    g_interfaces.global_vars->interpolation_amount = 0.f;

    player->set_abs_origin( record.origin );

    update_land( player, &record, previous );
    update_velocity( player, &record, previous );

    // fix various issues with the game
    // these issues can only occur when a player is choking data.
    if ( record.choked > 1 && previous && !previous->dormant ) {
        if ( record.on_ground )
            player->flags( ) |= player_flags::on_ground;
        else
            player->flags( ) &= ~player_flags::on_ground;
    }

    player->origin( ) = record.origin;
    player->velocity( ) = player->abs_velocity( ) = record.anim_velocity;
    player->eflags( ) &= ~( 0x1000 | 0x800 );

    const auto speed = math::length_2d( record.velocity );

    if ( ( record.flags & player_flags::on_ground ) && speed > 0.1f )
        record.mode = resolve_mode::walk;

    if ( ( record.flags & player_flags::on_ground ) && ( speed <= 0.1f ) )
        record.mode = resolve_mode::stand;

    else if ( !( record.flags & player_flags::on_ground ) )
        record.mode = resolve_mode::air;

    player->eye_angles( ) = record.eye_angles;

    if ( state->m_nLastUpdateFrame == g_interfaces.global_vars->framecount )
        state->m_nLastUpdateFrame = g_interfaces.global_vars->framecount - 1;

    if ( state->m_flLastUpdateTime == g_interfaces.global_vars->curtime )
        state->m_flLastUpdateTime = g_interfaces.global_vars->curtime - g_interfaces.global_vars->interval_per_tick;

    update_client_side_animations( player );

    player->flags( ) = backup_flags;
    player->eflags( ) = backup_eflags;
    player->origin( ) = backup_origin;
    player->velocity( ) = backup_velocity;
    player->abs_velocity( ) = backup_abs_velocity;
    player->set_abs_origin( backup_abs_origin );
    player->pose_parameters( ) = record.pose_parameters;
    player->set_abs_angles( vector_3d( 0.f, state->m_flFootYaw, 0.f ) );

    std::memcpy( player->anim_overlays( ), record.layer_records.data( ), record.layer_records.size( ) );
    std::memcpy( state, record.anim_state, sizeof( record.anim_state ) );

    g_interfaces.global_vars->restore( backup_vars );
    build_bones( player, record.bones.data( ), record.sim_time );
}

bool animation_sync::fix_velocity( c_cs_player *entity, vector_3d &velocity, const std::array< c_animation_layer, 15 > &anim_state, const vector_3d &origin ) {
    return true;
}

void animation_sync::clear_data( int index ) {
    if ( !index || index > 65 )
        return;

    if ( !lag_info[ index ].anim_records.empty( ) )
        lag_info[ index ].anim_records.clear( );
}

void animation_sync::should_interpolate( c_cs_player *player, bool state ) {
    auto map = player->get_var_mapping( );

    if ( map ) {
        for ( int i = 0; i < map->interpolated_entries; i++ ) {
            var_map_entry_t *e = &map->entries[ i ];

            e->needs_to_interpolate = state;
        }
    }
}

void animation_sync::on_net_update_postdataupdate_start( ) {
    if ( !g_interfaces.engine_client->is_in_game( ) || !g_interfaces.engine_client->is_connected( ) )
        return;

    if ( !globals::local_player )
        return;

    for ( int n = 1; n <= g_interfaces.global_vars->max_clients; ++n ) {
        const auto player = g_interfaces.entity_list->get_client_entity< c_cs_player * >( n );

        if ( !player || !player->is_player( ) || !player->alive( ) || player == globals::local_player )
            continue;

        /* remove interpolation and get last networked data from the entity before animating. */
        //should_interpolate( player, false );
    }
}

void animation_sync::on_pre_frame_render_start( ) {
    if ( !g_interfaces.engine_client->is_in_game( ) || !g_interfaces.engine_client->is_connected( ) )
        return;

    if ( !globals::local_player )
        return;

    for ( int n = 1; n <= ( g_interfaces.entity_list->get_highest_entity_index( ) + 1 ); ++n ) {
        const auto player = g_interfaces.entity_list->get_client_entity< c_cs_player * >( n );

        if ( !player || !player->alive( ) || !player->is_player( ) || player == globals::local_player )
            continue;

        auto &info = lag_info[ n ];

        if ( info.anim_records.empty( ) )
            continue;

        auto &front = info.anim_records.front( );

        animated_bones[ player->index( ) ] = front.bones;
        animated_origin[ player->index( ) ] = front.origin;
    }
}

void animation_sync::on_post_frame_render_start( ) {
    if ( !g_interfaces.engine_client->is_in_game( ) || !g_interfaces.engine_client->is_connected( ) )
        return;

    if ( !globals::local_player )
        return;

    static auto update_all_viewmodel_addons = signature::find( XOR( "client.dll" ), XOR( "E8 ? ? ? ? 5E 8B E5 5D C3 53 57" ) ).add( 0x1 ).rel32( ).get< void( __thiscall * )( void * ) >( );

    if ( globals::local_player->viewmodel_handle( ) != 0xFFFFFFFF ) {
        auto view_model = globals::local_player->get_view_model( );

        if ( view_model )
            update_all_viewmodel_addons( view_model );
    }
}

void animation_sync::on_net_update_end( ) {
    if ( !g_interfaces.engine_client->is_in_game( ) || !g_interfaces.engine_client->is_connected( ) )
        return;

    if ( !globals::local_player )
        return;

    auto net_channel = g_interfaces.engine_client->get_net_channel_info( );

    if ( !net_channel )
        return;

    for ( int n = 1; n <= ( g_interfaces.entity_list->get_highest_entity_index( ) + 1 ); ++n ) {
        const auto player = g_interfaces.entity_list->get_client_entity< c_cs_player * >( n );

        if ( !player || !player->is_player( ) || player == globals::local_player )
            continue;

        auto &info = lag_info[ n ];

        if ( !info.lag_records.empty( ) )
            info.lag_records.clear( );

        if ( !player->alive( ) ) {
            clear_data( n );
            continue;
        }

        if ( info.index != player->index( ) || player->spawn_time( ) != info.spawn_time )
            info.reset( player );

        info.anim_records.reserve( game::time_to_ticks( 1.0f ) );

        /* clear bad lag records */
        for ( int i = 0; i < info.anim_records.size( ); ++i ) {
            const auto &record = info.anim_records[ i ];

            /* first */
            if ( record.sim_time < g_interfaces.engine_client->get_last_time_stamp( ) - 1.0f ) {
                info.anim_records.current += ( info.anim_records.size( ) - i );
                info.anim_records.count -= ( info.anim_records.size( ) - i );
                break;
            }

            /* dead time (more expensive, but serves as a backup) */
            if ( record.sim_time < g_interfaces.engine_client->get_last_time_stamp( ) + game::ticks_to_time( net_channel->get_latency( 0 ) ) - globals::cvars::sv_maxunlag->get_float( ) ) {
                info.anim_records.current += ( info.anim_records.size( ) - i );
                info.anim_records.count -= ( info.anim_records.size( ) - i );
                break;
            }
        }

        if ( info.anim_records.exhausted( ) )
            info.anim_records.pop_back( );

        /* clears up more space on the stack if we make it static. */
        static lag_record record( player );

        if ( player->dormant( ) ) {
            bool insert = true;

            if ( !info.anim_records.empty( ) ) {
                auto &front = info.anim_records.front( );

                if ( front.dormant )
                    insert = false;
            }

            if ( insert ) {
                if ( auto *rec = info.anim_records.push_front( ) ) {
                    *rec = std::move( record );

                    rec->dormant = true;
                }
            }
        } else {
            bool should_update = ( info.anim_records.empty( ) || player->simtime( ) > player->old_simtime( ) );

            if ( should_update ) {
                if ( !info.anim_records.empty( ) && abs( game::time_to_ticks( player->simtime( ) - player->old_simtime( ) ) ) > 64 )
                    info.reset( player );

                lag_record *previous = nullptr;

                if ( !info.anim_records.empty( ) )
                    previous = &info.anim_records.front( );

                if ( auto *rec = info.anim_records.push_front( ) ) {
                    *rec = record;

                    rec->dormant = false;

                    update_player_animation( player, *rec, previous );
                }
            }
        }

        if ( !info.anim_records.empty( ) ) {
            for ( auto &rec : info.anim_records ) {
                if ( rec.is_valid( ) )
                    info.lag_records.emplace_back( &rec );
            }
        }

        ///* https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/game/server/player_lagcompensation.cpp#L246
        ///*/
        // int dead_time = globals::server_time - globals::cvars::sv_maxunlag->get_float();
        // int tail_index = lag_track[player->index()].size() - 1;

        // while (tail_index < lag_track[player->index()].size()) {
        //     const auto tail = lag_track[player->index()][tail_index];

        //    /* if tail is within limits, stop */
        //    if (tail->sim_time >= dead_time)
        //        break;

        //    /* remove tail, get new tail */
        //    lag_track[player->index()].pop_back();
        //    tail_index = lag_track[player->index()].size() - 1;
        // }
    }
}