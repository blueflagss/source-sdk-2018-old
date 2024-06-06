#include "anims.hpp"

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
    choked = std::clamp< int >( game::time_to_ticks( player->simtime( ) - old_sim_time ), 0, 16 );

    if ( player->bone_cache( ) )
        memcpy( this->bones.data( ), player->bone_cache( ), player->bone_count( ) * sizeof( matrix_3x4 ) );

    memcpy( this->pose_parameters.data( ), player->pose_parameters( ).data( ), sizeof( float ) * 24 );

    record_filled = true;
}

bool lag_record::is_valid( ) {
    if ( sim_time <= old_sim_time )
        return false;

    auto net_channel = g_interfaces.engine_client->get_net_channel_info( );

    if ( !net_channel )
        return false;

    const auto correct = std::clamp< float >( net_channel->get_latency( 1 ) + globals::lerp_amount, 0.0f, globals::cvars::sv_maxunlag->get_float( ) );

    return fabs( correct - ( g_interfaces.global_vars->curtime - this->sim_time ) ) <= 0.2f - game::ticks_to_time( 1 );
}

template< class T >
T animation_lerp( const T &current, const T &target, const int progress, const int max ) {
    return current + ( ( ( target - current ) / max ) * progress );
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
        state->update( player->eye_angles( ) );
        globals::allow_animations[ player->index( ) ] = false;
        player->client_side_animation( ) = last_client_side_anim;
    };

    c_global_vars_base backup_vars{ };
    backup_vars.backup( g_interfaces.global_vars );

    auto ticks_choked = record.choked;

    if ( previous ) {
        int simulation_ticks = game::time_to_ticks( player->simtime( ) - previous->sim_time );

        if ( simulation_ticks > 0 && simulation_ticks < 31 )
            ticks_choked = simulation_ticks;
    }

    player->set_abs_origin( record.origin );
    player->eflags( ) &= ~( 1 << 12 );

    state->m_flLastUpdateTime = player->old_simtime( );
    player->effects( ) &= ~0x1800;// force velocity.

    const auto speed = math::length_2d( record.velocity );

    if ( ( record.flags & player_flags::on_ground ) && speed > 0.1f )
        record.mode = resolve_mode::walk;

    if ( ( record.flags & player_flags::on_ground ) && ( speed <= 0.1f ) )
        record.mode = resolve_mode::stand;

    else if ( !( record.flags & player_flags::on_ground ) )
        record.mode = resolve_mode::air;

    if ( record.choked <= 1 || !previous ) {
        auto current_simulation_tick = game::time_to_ticks( record.sim_time );

        g_interfaces.global_vars->curtime = record.sim_time;
        g_interfaces.global_vars->frametime = g_interfaces.global_vars->interval_per_tick;
        g_interfaces.global_vars->realtime = record.sim_time;
        g_interfaces.global_vars->abs_frametime = g_interfaces.global_vars->interval_per_tick;
        g_interfaces.global_vars->framecount = current_simulation_tick;
        g_interfaces.global_vars->tick_count = current_simulation_tick;
        g_interfaces.global_vars->interpolation_amount = 0.f;

        player->eye_angles( ) = record.eye_angles;
        player->velocity( ) = record.velocity;
        player->abs_velocity( ) = record.velocity;

        update_client_side_animations( player );
    }

    else {
        for ( int simulation_tick = 1; simulation_tick <= record.choked; simulation_tick++ ) {
            auto simtime = previous->sim_time + game::ticks_to_time( simulation_tick );
            auto current_simulation_tick = game::ticks_to_time( simtime );

            g_interfaces.global_vars->curtime = simtime;
            g_interfaces.global_vars->realtime = simtime;
            g_interfaces.global_vars->frametime = g_interfaces.global_vars->interval_per_tick;
            g_interfaces.global_vars->abs_frametime = g_interfaces.global_vars->interval_per_tick;
            g_interfaces.global_vars->framecount = current_simulation_tick;
            g_interfaces.global_vars->tick_count = current_simulation_tick;

            player->origin( ).x = animation_lerp< float >( previous->origin.x, record.origin.x, simulation_tick, record.choked );
            player->origin( ).y = animation_lerp< float >( previous->origin.y, record.origin.y, simulation_tick, record.choked );
            player->origin( ).z = animation_lerp< float >( previous->origin.z, record.origin.z, simulation_tick, record.choked );

            player->set_abs_origin( player->origin( ) );

            auto &velocity = player->velocity( );

            velocity.x = animation_lerp< float >( previous->velocity.x, record.velocity.x, simulation_tick, record.choked );
            velocity.y = animation_lerp< float >( previous->velocity.y, record.velocity.y, simulation_tick, record.choked );

            player->abs_velocity( ) = velocity;

            update_client_side_animations( player );
        }
    }

    record.has_velocity = fix_velocity( player, player->velocity( ), record.layer_records, record.origin );

    player->flags( ) = backup_flags;
    player->eflags( ) = backup_eflags;
    player->origin( ) = backup_origin;
    player->velocity( ) = backup_velocity;
    player->abs_velocity( ) = backup_abs_velocity;
    player->set_abs_origin( backup_abs_origin );
    player->pose_parameters( ) = record.pose_parameters;

    std::memcpy( player->anim_overlays( ), record.layer_records.data( ), record.layer_records.size( ) );
    player->anim_state( ) = record.anim_state;

    g_interfaces.global_vars->restore( backup_vars );
    build_bones( player, record.bones.data( ), record.sim_time );
    player->invalidate_physics_recursive( invalidate_physics_bits::animation_changed | invalidate_physics_bits::position_changed | invalidate_physics_bits::velocity_changed );
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

    auto view_model = globals::local_player->get_view_model( );

    if ( view_model )
        update_all_viewmodel_addons( view_model );
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