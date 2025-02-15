#include "animation_sync.hpp"
#include "animation_state_rebuilt.hpp"
#include <core/config.hpp>
#include <features/bone_setup/bone_setup.hpp>
#include <features/prediction_manager/prediction_manager.hpp>
#include <features/ragebot/ragebot.hpp>
#include <features/resolver/resolver.hpp>
#include <features/sound_handler/sound_handler.hpp>
#include <threadutils/threading.h>
#include <DirectXMath.h>

void lag_record::reset( c_cs_player *player ) {
    this->player = player;

    renderable_to_world_transform = player->renderable_to_world_transform( );
    flags = player->flags( );
    origin = player->origin( );
    mins = player->collideable( )->mins( );
    maxs = player->collideable( )->maxs( );
    duck_amount = player->duck_amount( );
    lower_body_yaw = player->lower_body_yaw_target( );
    sim_time = player->simtime( );
    old_sim_time = player->old_simtime( );
    abs_angles = player->get_abs_angles( );
    abs_origin = player->get_abs_origin( );
    velocity = player->velocity( );
    index = player->index( );
    eye_angles = player->eye_angles( );
    model = player->get_model( );
    tick = game::time_to_ticks( g_interfaces.engine_client->get_last_time_stamp( ) );
    anim_time = player->old_simtime( ) + g_interfaces.global_vars->interval_per_tick;
    choked = std::clamp< int >( game::time_to_ticks( player->simtime( ) - old_sim_time ), 0, 20 );
    on_ground = false;
    is_exploit = false;
    forward_track = false;
    mode = resolve_mode::none;

    if ( player->bone_cache( ) )
        std::memcpy( bones.data( ), player->bone_cache( ), sizeof( matrix_3x4 ) * player->bone_count( ) );

    record_filled = true;
}

bool lag_record::is_valid( ) {
    auto net_channel = g_interfaces.engine_client->get_net_channel_info( );

    if ( !net_channel )
        return false;

    const float curtime = globals::local_player->alive( ) ? game::ticks_to_time( globals::local_player->tick_base( ) ) : g_interfaces.global_vars->curtime;

    float correct = 0.0f;

    correct += net_channel->get_latency( 1 );
    correct += ( g_vars.misc_fake_latency.value && globals::hotkeys::fake_latency ) ? g_vars.misc_fake_latency_value.value : 0.0f;
    correct += globals::lerp_amount;
    correct = std::clamp< float >( correct, 0.0f, globals::cvars::sv_maxunlag->get_float( ) );

    const int dead_time = curtime - globals::cvars::sv_maxunlag->get_float( );

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

void animation_sync::generate_shoot_position( ) {
}

struct movement_args {
    c_cs_player *player;
    vector_3d origin;
    vector_3d velocity;
};

static void threaded_process_movement( movement_args *args ) {
    if ( !args || !args->player )
        return;

    g_prediction_context.simulate_tick( );

    args->velocity = args->player->velocity( );
    args->origin = args->player->origin( );
}

bool animation_sync::should_predict_lag( aim_player &target, lag_record *record, lag_record *previous ) {
    target.delay_shot = false;

    if ( !record )
        return false;

    auto player = target.entity;

    if ( !player )
        return false;

    if ( record->choked <= 1 )
        return false;

    auto anim_state = player->anim_state( );

    if ( !anim_state )
        return false;

    auto &info = player_log[ target.entity->index( ) ];

    if ( info.anim_records.empty( ) )
        return false;

    size_t size{ };

    for ( const auto &it : info.anim_records ) {
        if ( it.dormant )
            break;
        ++size;
    }

    const auto prev_1 = ( &info.anim_records[ info.lag_records.size( ) - 1 ] );
    const auto prev_2 = ( &info.anim_records[ info.lag_records.size( ) - 2 ] );

    if ( !prev_1 || !prev_2 )
        return false;

    if ( size > 1 && ( math::length_sqr( record->origin - prev_1->origin ) > 4096.f || size > 2 && math::length_sqr( prev_1->origin - prev_2->origin ) > 4096.f ) )
        record->break_lc = true;

    if ( !record->break_lc )
        return false;

    int shot = 0;
    int pred = 0;

    if ( g_vars.aimbot_fix_fakelag.value ) {
        const auto lag_amount = std::clamp< int >( record->choked, 1, 16 );

        if ( !g_prediction_context.predict_player_entity( record->player ) )
            return false;

        int next_tick = g_interfaces.client_state->server_tick( ) + 1;
        const int cmd_arrival_tick = g_interfaces.client_state->server_tick( ) + globals::latency_ticks;

        lag_record *newest_non_onshot_rec = nullptr;
        float newest_time = 0.0f;

        for ( auto &rec : info.anim_records ) {
            if ( rec.sim_time > newest_time && !rec.shot ) {
                newest_non_onshot_rec = &rec;
                newest_time = rec.sim_time;
            }
        }

        auto newest_rec_for_bones = newest_non_onshot_rec ? newest_non_onshot_rec : record;
        auto newest_rec = record;
        auto hit_tick = game::time_to_ticks( newest_rec->sim_time );
        auto max_lag_choke = std::clamp< int >( newest_rec->choked + 1, 1, 16 );

        if ( next_tick + max_lag_choke < cmd_arrival_tick ) {
            while ( true ) {
                next_tick += max_lag_choke;

                if ( next_tick >= cmd_arrival_tick )
                    break;

                auto extrap_rec = new lag_record{ *newest_rec };

                const auto start_origin = newest_rec_for_bones->origin;
                extrap_rec->bones = newest_rec_for_bones->bones;

                for ( int sim = 1; sim < max_lag_choke; sim++ ) {
                    movement_args mv;
                    mv.player = record->player;
                    extrapolate( extrap_rec, extrap_rec->origin, extrap_rec->velocity, extrap_rec->flags, player->flags( ) & player_flags::on_ground );
                    hit_tick++;

                    if ( mv.origin != vector_3d( ) )
                        extrap_rec->origin = mv.origin;
                    if ( sim == max_lag_choke ) {
                        break;
                    }
                }

                for ( auto &mat : extrap_rec->bones )
                    mat.set_origin( mat.get_origin( ) - start_origin + extrap_rec->origin );

                extrap_rec->sim_time = extrap_rec->anim_time = game::ticks_to_time( hit_tick );
                extrap_rec->forward_track = true;
                extrap_rec->break_lc = false;

                *record = *extrap_rec;

                if ( pred > 4 )
                    break;

                pred++;
            }

            g_prediction_context.restore( );
        }
    }

    const auto tick_base = game::time_to_ticks( record->sim_time );

    if ( std::abs( globals::arrival_tick - tick_base ) >= std::round( 1.f / g_interfaces.global_vars->interval_per_tick ) ) {
        target.delay_shot = true;
        return true;
    }

    int simulation = game::time_to_ticks( record->sim_time );

    if ( std::abs( globals::arrival_tick - simulation ) >= 128 )
        return true;

    const auto receive_tick = std::abs( globals::arrival_tick - game::time_to_ticks( record->sim_time ) );

    if ( receive_tick / record->choked > 19 ) {
        target.delay_shot = true;
        return true;
    }

    const auto adjusted_arrive_tick = game::time_to_ticks( globals::latency + g_interfaces.global_vars->realtime - game::ticks_to_time( record->tick ) );

    record->arrival_tick = adjusted_arrive_tick;

    if ( adjusted_arrive_tick - record->choked >= 0 ) {
        target.delay_shot = true;
        return true;
    }

    return true;
}

bool animation_sync::get_lagcomp_bones( c_cs_player *player, std::array< matrix_3x4, 128 > &out ) {
    auto net_channel = g_interfaces.engine_client->get_net_channel_info( );

    if ( !net_channel )
        return false;

    auto time_valid_no_deadtime = [ & ]( float t ) {
        const auto correct = std::clamp< float >( net_channel->get_latency( 1 ) + net_channel->get_latency( 0 ) + globals::lerp_amount, 0.0f, globals::cvars::sv_maxunlag->get_float( ) );
        return fabs( correct - ( g_interfaces.global_vars->curtime - t ) ) <= 0.2f;
    };

    auto &log = g_animations.player_log[ player->index( ) ];

    if ( !log.anim_records.empty( ) ) {
        const auto lag_records = log.anim_records;

        for ( int i = static_cast< int >( lag_records.size( ) ) - 1; i >= 0; i-- ) {
            auto &record = lag_records[ i ];

            if ( time_valid_no_deadtime( lag_records[ i ].sim_time ) ) {
                if ( glm::length( record.origin - player->origin( ) ) < 1.0f )
                    return false;

                bool end = ( i - 1 ) <= 0;
                vector_3d next = end ? player->get_abs_origin( ) : lag_records[ i - 1 ].abs_origin;
                float time_next = end ? player->simtime( ) : lag_records[ i - 1 ].sim_time;

                float correct = net_channel->get_latency( 0 ) + net_channel->get_latency( 1 ) + globals::lerp_amount;
                float time_delta = time_next - lag_records[ i ].sim_time;
                float add = end ? 0.2f : time_delta;
                float deadtime = lag_records[ i ].sim_time;

                float curtime = g_interfaces.global_vars->curtime;
                float delta = deadtime - curtime;

                float mul = 1.f / add;

                auto lerp = next + ( lag_records[ i ].abs_origin - next ) * std::clamp( delta * mul, 0.0f, 1.0f );

                out = lag_records[ i ].bones;

                for ( auto &iter : out )
                    iter.set_origin( iter.get_origin( ) - lag_records[ i ].abs_origin + lerp );

                return true;
            }
        }
    }

    return false;
};

bool animation_sync::build_bones( c_cs_player *player, matrix_3x4 *out, std::array< float, 24 > &poses, float curtime ) {
    const auto backup_effects = player->effects( );
    const auto backup_setup_time = player->last_bone_setup_time( );

    player->last_bone_setup_time( ) = 0;
    player->effects( ) |= effects::nointerp;

    bool setup = player->setup_bones( out, 128, 0x7FF00, curtime );

    player->last_bone_setup_time( ) = backup_setup_time;
    player->effects( ) = backup_effects;

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
        if ( record->layer_records[ ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ].weight != 1.f &&
             record->layer_records[ ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ].weight == 1.f &&
             record->layer_records[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ].weight != 0.f ) {
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
    auto state = player->anim_state( );

    if ( !state || !previous )
        return;

    auto weapon = g_interfaces.entity_list->get_client_entity_from_handle< c_cs_weapon_base * >( player->weapon_handle( ) );

    if ( !weapon )
        return;

    record->fake_walk = false;

    if ( record->choked > 0 && record->choked < 16 && previous ) {
        if ( previous && !previous->dormant )
            record->velocity = ( record->origin - previous->origin ) * ( 1.0f / game::ticks_to_time( record->choked ) );
    }

    if ( record->flags & player_flags::on_ground && ( glm::length( record->velocity ) > 0.1f || state->m_flVelocityLengthXY > 0.1f ) && record->layer_records[ ANIMATION_LAYER_LEAN ].weight == 0.f && record->layer_records[ ANIMATION_LAYER_MOVEMENT_MOVE ].weight < 0.1f )
        record->fake_walk = true;

    record->velocity_detail = fix_velocity( record->layer_records.data( ), previous, player );
    player->velocity( ) = player->abs_velocity( ) = record->velocity;

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

void animation_sync::copy_layers( c_animation_layer *dst, c_animation_layer *src ) {
    for ( int i = 0; i < ANIMATION_LAYER_COUNT; i++ ) {
        if ( i == ANIMATION_LAYER_ADJUST || i == ANIMATION_LAYER_FLASHED || i == ANIMATION_LAYER_WHOLE_BODY || i == ANIMATION_LAYER_MOVEMENT_MOVE || i == ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ) {
            dst[ i ].order = src[ i ].order;
            dst[ i ].sequence = src[ i ].sequence;
            dst[ i ].previous_cycle = src[ i ].previous_cycle;
            dst[ i ].weight = src[ i ].weight;
            dst[ i ].weight_delta_rate = src[ i ].weight_delta_rate;
            dst[ i ].playback_rate = src[ i ].playback_rate;
            dst[ i ].cycle = src[ i ].cycle;
        }
    }
}

void animation_sync::update_local_animations( c_user_cmd *user_cmd ) {
    globals::lby_updating = false;

    if ( !globals::local_player )
        return;
    auto state = globals::local_player->anim_state( );

    if ( !state )
        return;

    if ( !g_interfaces.client_state->choked_commands( ) ) {
        if ( !on_ground && state->m_bOnGround ) {
            body = globals::sent_angles.y;
            lower_body_realign_timer = g_interfaces.global_vars->curtime;
        }

        if ( state->m_bOnGround ) {
            if ( state->m_flVelocityLengthXY > 0.1f ) {
                lower_body_realign_timer = g_interfaces.global_vars->curtime + 0.22f;
            } else if ( g_interfaces.global_vars->curtime > lower_body_realign_timer ) {
                globals::lby_updating = true;
                lower_body_realign_timer = g_interfaces.global_vars->curtime + 1.1f;
            }
        }
    }

    if ( !*globals::packet ) {
        return;
    }

    c_global_vars_base global_vars{ };

    global_vars.backup( g_interfaces.global_vars );

    std::memcpy( pose_parameters.data( ), globals::local_player->pose_parameters( ).data( ), sizeof( pose_parameters ) );
    foot_yaw = state->m_flFootYaw;

    globals::local_player->player_state( ).v_angle = user_cmd->view_angles;
    std::memcpy( animation_layers.data( ), globals::local_player->anim_overlays( ), sizeof( animation_layers ) );

    const auto client_side_backup = globals::local_player->client_side_animation( );

    g_interfaces.global_vars->frametime = g_interfaces.global_vars->interval_per_tick;

    state->m_nLastUpdateFrame = 0;

    auto angles = globals::lby_updating ? globals::sent_user_cmd.view_angles : globals::angles;

    globals::allow_animations[ globals::local_player->index( ) ] = globals::local_player->client_side_animation( ) = true;
    g_rebuilt.update_animation_state( state, angles, true );
    globals::local_player->client_side_animation( ) = globals::allow_animations[ globals::local_player->index( ) ] = client_side_backup;

    g_interfaces.global_vars->restore( global_vars );

    if ( g_config.get_hotkey( g_vars.misc_fake_walk_key, g_vars.misc_fake_walk_key_toggle.value ) )
        animation_layers[ ANIMATION_LAYER_MOVEMENT_MOVE ].weight = 0.0f;

    if ( globals::local_player->anim_overlays( ) ) {
        for ( int i = 0; i < ANIMATION_LAYER_COUNT; i++ )
            globals::local_player->anim_overlays( )[ i ].owner = globals::local_player;
    }

    const auto backup_angle = globals::local_player->player_state( ).v_angle;

    if ( globals::local_player->sequence( ) != -1 )
        utils::get_method< void( __thiscall * )( void *, int ) >( globals::local_player, 107 )( globals::local_player, 1 );

    if ( *reinterpret_cast< bool * >( reinterpret_cast< uintptr_t >( globals::local_player ) + 0x99CD ) )
        g_addresses.handle_taser_anim.get< void( __thiscall * )( void * ) >( )( globals::local_player );

    animated_origin[ globals::local_player->index( ) ] = globals::local_player->origin( );

    g_bone_setup.build( globals::local_player, animated_bones[ globals::local_player->index( ) ].data( ), bone_used_by_anything, globals::local_player->origin( ), globals::local_player->get_abs_angles( ), g_interfaces.global_vars->curtime, angles );

    on_ground = state->m_bOnGround;
    globals::local_player->player_state( ).v_angle = backup_angle;
}

void animation_sync::maintain_local_animations( ) {
    if ( !globals::local_player )
        return;

    auto state = globals::local_player->anim_state( );

    if ( !state )
        return;

    globals::local_player->set_abs_angles( vector_3d( 0.f, foot_yaw, 0.f ) );

    if ( g_interfaces.input->camera_in_thirdperson )
        g_interfaces.prediction->set_local_view_angles( radar_angle );
}

void animation_sync::update_player_animation( c_cs_player *player, lag_record &record, lag_record *previous, bool update ) {
    const auto state = player->anim_state( );

    if ( !state )
        return;

    record.anim_state = *state;
    record.pose_parameters = player->pose_parameters( );


    if ( update ) {
        if ( previous ) {
            if ( math::length_sqr( record.origin - previous->origin ) > 4096.f )
                record.break_lc = true;
        }

        if ( record.break_lc )
            clear_data( player->index( ) );
    }

    std::memcpy( record.layer_records.data( ), player->anim_overlays( ), record.layer_records.size( ) );

    const auto backup_simtime = player->simtime( );
    const auto backup_velocity = player->velocity( );
    const auto backup_abs_velocity = player->abs_velocity( );
    const auto backup_abs_origin = player->get_abs_origin( );
    const auto backup_origin = player->origin( );
    const auto backup_flags = player->flags( );
    const auto backup_lower_body_yaw = player->lower_body_yaw_target( );
    const auto backup_eflags = player->eflags( );
    const auto backup_abs_angles = player->get_abs_angles( );
    const auto backup_duck_amount = player->duck_amount( );
     
    const auto &log = player_log[ player->index( ) ];
    const auto enemy = globals::local_player->team( ) != player->team( );

    const float time = ( record.choked >= 10 ) ? record.sim_time : record.anim_time;

    const auto backup_curtime = g_interfaces.global_vars->curtime;
    const auto backup_frametime = g_interfaces.global_vars->frametime;

	g_interfaces.global_vars->frametime = game::ticks_to_time( 1 );
    g_interfaces.global_vars->curtime = player->simtime( );

    player->set_abs_origin( record.origin );
    player->origin( ) = record.origin;
    player->eflags( ) &= ~0x1000;

    if ( update ) {
        const auto should_resolve = !log.player_info.fake_player && player->team( ) != globals::local_player->team( );

        if ( should_resolve ) {
            update_land( player, &record, previous );
            update_velocity( player, &record, previous );

            g_resolver.start( player, record, previous );
            math::angle_normalize( player->eye_angles( ).y );
            record.eye_angles = player->eye_angles( );

            if ( record.choked > 1 && previous && !previous->dormant ) {
                if ( record.on_ground )
                    player->flags( ) |= player_flags::on_ground;
                else
                    player->flags( ) &= ~player_flags::on_ground;
            }
        }

        const auto backup_clientside = player->client_side_animation( );

        if ( state->m_nLastUpdateFrame == g_interfaces.global_vars->framecount )
            state->m_nLastUpdateFrame = g_interfaces.global_vars->framecount - 1;

        if ( state->m_flLastUpdateTime == g_interfaces.global_vars->curtime )
            state->m_flLastUpdateTime = g_interfaces.global_vars->curtime - g_interfaces.global_vars->interval_per_tick;

        globals::allow_animations[ player->index( ) ] = player->client_side_animation( ) = true;
        player->update_clientside_animation( );

        //g_rebuilt.update_animation_state( state, player->eye_angles( ), true );
        globals::allow_animations[ player->index( ) ] = false;
        player->client_side_animation( ) = backup_clientside;

        //player->invalidate_physics_recursive( angles_changed );
        //player->invalidate_physics_recursive( animation_changed );
        //player->invalidate_physics_recursive( sequence_changed );
    }



    player->flags( ) = backup_flags;
    player->eflags( ) = backup_eflags;
    player->origin( ) = backup_origin;
    player->velocity( ) = backup_velocity;
    player->abs_velocity( ) = backup_abs_velocity;
    player->duck_amount( ) = backup_duck_amount;
    player->set_abs_origin( backup_abs_origin );
    player->eye_angles( ) = record.eye_angles;
    std::memcpy( player->anim_overlays( ), record.layer_records.data( ), record.layer_records.size( ) );
    player->pose_parameters( ) = record.pose_parameters;
    player->set_abs_angles( vector_3d( 0.f, state->m_flFootYaw, 0.f ) );
    //*player->anim_state( ) = record.anim_state;

    g_interfaces.global_vars->curtime = backup_curtime;
    g_interfaces.global_vars->frametime = backup_frametime;
}

velocity_detail animation_sync::fix_velocity( c_animation_layer *layers, lag_record *previous, c_cs_player *player ) {
    if ( !layers )
        return velocity_detail::none;

    const auto &jump_or_fall_layer = layers[ ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ];
    const auto &move_layer = layers[ ANIMATION_LAYER_MOVEMENT_MOVE ];
    const auto &land_or_climb_layer = layers[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ];
    const auto &alive_loop_layer = layers[ ANIMATION_LAYER_ALIVELOOP ];

    const bool on_ground = ( player->flags( ) & player_flags::on_ground ) != 0;
    vector_3d &velocity = player->velocity( );

    if ( on_ground && move_layer.weight == 0.0f ) {
        velocity = { };
        return velocity_detail::zero;
    }

    const float dt = std::max( g_interfaces.global_vars->interval_per_tick, player->simtime( ) - previous->old_sim_time );
    vector_3d avg_velocity = ( player->origin( ) - previous->origin ) / dt;

    if ( math::length_2d( previous->velocity ) <= 0.1f || dt <= g_interfaces.global_vars->interval_per_tick ) {
        velocity = { };
        return velocity_detail::perfect;
    }

    const auto active_weapon = g_interfaces.entity_list->get_client_entity_from_handle< c_cs_weapon_base * >( player->weapon_handle( ) );

    if ( !active_weapon ) {
        velocity = { };
        return velocity_detail::zero;
    }

    const auto *weapon_info = active_weapon ? active_weapon->get_weapon_data( ) : nullptr;
    float max_weapon_speed = weapon_info ? ( player->scoped( ) ? weapon_info->max_speed : weapon_info->max_speed_alt ) : 250.0f;

    if ( on_ground ) {
        velocity = avg_velocity;

        auto detail = velocity_detail::none;

        if ( move_layer.playback_rate > 0.0f ) {
            auto direction = math::normalize_angle( velocity );
            const float avg_speed_xy = math::length_2d( velocity );
            const float move_weight_with_air_smooth = move_layer.weight;
            const float target_move_weight_to_speed_xy = max_weapon_speed * valve_math::Lerp( CS_PLAYER_SPEED_WALK_MODIFIER, CS_PLAYER_SPEED_DUCK_MODIFIER, player->duck_amount( ) ) * move_weight_with_air_smooth;
            const float speed_as_portion_of_run_top_speed = 0.35f * ( 1.0f - alive_loop_layer.weight );

            if ( alive_loop_layer.weight > 0.0f && alive_loop_layer.weight < 1.0f ) {
                const float speed_xy = max_weapon_speed * ( speed_as_portion_of_run_top_speed + 0.55f );
                velocity = direction * speed_xy;
                detail = velocity_detail::running;
            } else if ( move_weight_with_air_smooth < 0.95f || target_move_weight_to_speed_xy > avg_speed_xy ) {
                velocity = direction * target_move_weight_to_speed_xy;

                const float prev_move_weight = previous->layer_records[ ANIMATION_LAYER_MOVEMENT_MOVE ].weight;
                const float weight_delta_rate = ( move_layer.weight - prev_move_weight ) / dt;
                const bool walking_speed = math::length_2d( velocity ) > max_weapon_speed * CS_PLAYER_SPEED_WALK_MODIFIER;
                const bool constant_speed = abs( weight_delta_rate ) < ( walking_speed ? 0.9f : 0.15f );

                if ( move_layer.weight == prev_move_weight )
                    detail = velocity_detail::constant;
                else if ( move_layer.weight > prev_move_weight )
                    detail = velocity_detail::accelerating;
            } else {
                float target_move_weight_adjusted_speed_xy = max_weapon_speed * move_weight_with_air_smooth;

                if ( ( player->flags( ) & player_flags::ducking ) != 0 )
                    target_move_weight_adjusted_speed_xy *= CS_PLAYER_SPEED_DUCK_MODIFIER;
                else if ( player->walking( ) )
                    target_move_weight_adjusted_speed_xy *= CS_PLAYER_SPEED_WALK_MODIFIER;

                const float prev_move_weight = previous->layer_records[ ANIMATION_LAYER_MOVEMENT_MOVE ].weight;

                if ( avg_speed_xy > target_move_weight_adjusted_speed_xy ) {
                    velocity = direction * target_move_weight_adjusted_speed_xy;

                    const float weight_delta_rate = ( move_layer.weight - prev_move_weight ) / dt;
                    const bool walking_speed = math::length_2d( velocity ) > max_weapon_speed * CS_PLAYER_SPEED_WALK_MODIFIER;
                    const bool constant_speed = abs( weight_delta_rate ) < ( walking_speed ? 0.9f : 0.15f );
                }

                if ( move_layer.weight == prev_move_weight )
                    detail = velocity_detail::constant;
                else if ( move_layer.weight > prev_move_weight )
                    detail = velocity_detail::accelerating;
            }
        }

        return detail;
    } else {
        int seq = -1;

        const bool crouch = player->duck_amount( ) > 0.0f;
        const float speed_as_portion_of_walk_top_speed = math::length_2d( avg_velocity ) / ( max_weapon_speed * CS_PLAYER_SPEED_WALK_MODIFIER );
        const bool moving = speed_as_portion_of_walk_top_speed > 0.25f;

        seq = moving + 17;
        if ( !crouch )
            seq = moving + 15;

        velocity = avg_velocity;

        if ( jump_or_fall_layer.weight > 0.0f && jump_or_fall_layer.playback_rate > 0.0f && jump_or_fall_layer.sequence == seq ) {
            const float time_since_jump = jump_or_fall_layer.cycle * jump_or_fall_layer.playback_rate;
            velocity.z = globals::cvars::sv_jump_impulse->get_float( ) - time_since_jump * globals::cvars::sv_gravity->get_float( ) * 0.5f;
        }

        return velocity_detail::perfect;
    }

    player->velocity( ) = velocity;

    return velocity_detail::none;
}

bool animation_sync::fix_velocity( c_cs_player *entity, vector_3d &velocity, const std::array< c_animation_layer, 15 > &anim_state, const vector_3d &origin ) {
    return true;
}

void animation_sync::clear_data( int index ) {
    if ( !index || index > 65 )
        return;

    if ( !player_log[ index ].anim_records.empty( ) )
        player_log[ index ].anim_records.clear( );

    if ( !player_log[ index ].lag_records.empty( ) )
        player_log[ index ].lag_records.clear( );
}

void animation_sync::on_post_frame_stage_notify( client_frame_stage stage ) {
    if ( !g_interfaces.engine_client->is_in_game( ) || !g_interfaces.engine_client->is_connected( ) )
        return;

    if ( !globals::local_player )
        return;

    if ( stage == client_frame_stage::frame_net_update_end )
        on_net_update_end( );

    if ( stage == client_frame_stage::frame_render_start && globals::local_player->alive( ) ) {
        static auto update_all_viewmodel_addons = signature::find( _xs( "client.dll" ), _xs( "E8 ? ? ? ? 5E 8B E5 5D C3 53 57" ) ).add( 0x1 ).rel32( ).get< void( __thiscall * )( void * ) >( );

        maintain_local_animations( );

        static auto draw_server_hitboxes = signature::find( _xs( "server.dll" ), _xs( "E8 ? ? ? ? F6 83 ? ? ? ? ? 0F 84 ? ? ? ? 33 FF 39 BB" ) ).add( 0x1 ).rel32( ).get< void * >( );
        static auto util_player_by_index = signature::find( _xs( "server.dll" ), _xs( "85 C9 7E 2A A1" ) ).get< c_cs_player *( __fastcall * ) ( int ) >( );

        if ( g_vars.visuals_other_show_server_hitboxes.value ) {
            float duration = -1.0f;

            for ( int i = 1; i < g_interfaces.global_vars->max_clients; i++ ) {
                auto e = g_interfaces.entity_list->get_client_entity< c_cs_player * >( i );
                if ( !e )
                    continue;

                auto ent = util_player_by_index( e->index( ) );
                if ( !ent )
                    continue;

                __asm {
                    pushad
                    movss xmm1, duration
                    push 1
                    mov ecx, ent
                    call draw_server_hitboxes
                    popad
                }
            }
        }

        if ( globals::local_player->viewmodel_handle( ) != -1 ) {
            const auto view_model = globals::local_player->get_view_model( );

            if ( view_model )
                update_all_viewmodel_addons( view_model );
        }
    }
}

void animation_sync::extrapolate( lag_record *record, vector_3d &origin, vector_3d &velocity, int &flags, bool on_ground ) {
    if ( !( flags & player_flags::on_ground ) )
        velocity.z -= game::ticks_to_time( globals::cvars::sv_gravity->get_float( ) );
    else if ( ( record->flags & player_flags::on_ground ) && !on_ground )
        velocity.z = globals::cvars::sv_jump_impulse->get_float( );

    const auto src = origin;
    auto end = src + velocity * g_interfaces.global_vars->interval_per_tick;

    ray_t ray;
    ray.init( src, end, record->mins, record->maxs );

    c_game_trace t;
    c_trace_filter_world_and_props filter;

    g_interfaces.engine_trace->trace_ray( ray, mask_playersolid, &filter, &t );

    if ( t.fraction != 1.f ) {
        for ( auto i = 0; i < 2; i++ ) {
            velocity -= t.plane.normal * glm::dot( velocity, t.plane.normal );
            const auto dot = glm::dot( velocity, t.plane.normal );
            if ( dot < 0.f )
                velocity -= vector_3d{ dot * t.plane.normal.x, dot * t.plane.normal.y, dot * t.plane.normal.z };

            end = t.end_pos + velocity * game::ticks_to_time( 1.f - t.fraction );
            ray_t extrap_ray;
            extrap_ray.init( t.end_pos, end, record->mins, record->maxs );

            g_interfaces.engine_trace->trace_ray( extrap_ray, mask_playersolid, &filter, &t );

            if ( t.fraction == 1.f )
                break;
        }
    }

    origin = end = t.end_pos;
    end.z -= 2.f;

    ray_t last_ray;
    last_ray.init( origin, end, record->mins, record->maxs );
    g_interfaces.engine_trace->trace_ray( last_ray, mask_playersolid, &filter, &t );

    flags &= ~player_flags::on_ground;

    if ( t.fraction != 1.0f && t.plane.normal.z > .7f )
        flags |= player_flags::on_ground;
}

void animation_sync::on_pre_frame_stage_notify( client_frame_stage stage ) {
    if ( !g_interfaces.engine_client->is_in_game( ) || !g_interfaces.engine_client->is_connected( ) )
        return;

    if ( !globals::local_player || !globals::local_player->anim_overlays( ) )
        return;

    if ( !globals::local_player->alive( ) )
        memcpy( queued_animation_layers.data( ), globals::local_player->anim_overlays( ), queued_animation_layers.size( ) );

    if ( stage == client_frame_stage::frame_render_start ) {
        apply_animations( );
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

    if ( globals::local_player->anim_overlays( ) )
        std::memcpy( playback_animation_layers.data( ), globals::local_player->anim_overlays( ), playback_animation_layers.size( ) );

    for ( int n = 1; n <= ( g_interfaces.entity_list->get_highest_entity_index( ) + 1 ); ++n ) {
        const auto player = g_interfaces.entity_list->get_client_entity< c_cs_player * >( n );
        if ( !player || !player->is_player( ) || player == globals::local_player )
            continue;

        if ( !player->alive( ) ) {
            clear_data( n );
            continue;
        }

        auto state = player->anim_state( );

        if ( !state ) {
            clear_data( n );
            continue;
        }

        auto &info = player_log[ n ];

        if ( info.index != player->index( ) || player->spawn_time( ) != info.spawn_time )
            info.reset( player );

        for ( int i = 0; i < info.anim_records.size( ); ++i ) {
            const auto &record = info.anim_records[ i ];

            if ( record.sim_time < g_interfaces.engine_client->get_last_time_stamp( ) - 1.0f ) {
                info.anim_records.current += ( info.anim_records.size( ) - i );
                info.anim_records.count -= ( info.anim_records.size( ) - i );
                break;
            }

            if ( record.sim_time < g_interfaces.engine_client->get_last_time_stamp( ) + game::ticks_to_time( net_channel->get_latency( 0 ) ) - globals::cvars::sv_maxunlag->get_float( ) ) {
                info.anim_records.current += ( info.anim_records.size( ) - i );
                info.anim_records.count -= ( info.anim_records.size( ) - i );
                break;
            }
        }

        info.anim_records.reserve( globals::tick_rate );

        if ( info.anim_records.exhausted( ) )
            info.anim_records.pop_back( );

        lag_record record( player );

        if ( record.dormant && player->dormant( ) ) {
            bool insert = true;

            if ( g_sound_handler.dormant_players[ player->index( ) ].visible )
                g_sound_handler.update_position( player );

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
            bool next_update = player->simtime( ) > player->old_simtime( );

            record.break_lc = false;

            if ( record.sim_time <= record.old_sim_time )
                record.break_lc = true;

            if ( ( record.choked <= 5 && record.choked <= 17 ) && player->simtime( ) == player->old_simtime( ) ) {
                record.is_exploit = true;
            }

            bool should_update = ( info.anim_records.empty( ) || next_update );

            lag_record *previous = nullptr;

            if ( should_update ) {
                if ( !info.anim_records.empty( ) && abs( game::time_to_ticks( player->simtime( ) - player->old_simtime( ) ) ) > 64 )
                    info.reset( player );

                if ( !info.anim_records.empty( ) )
                    previous = &info.anim_records.front( );

                if ( auto *rec = info.anim_records.push_front( ) ) {
                    *rec = record;
                    rec->reset( player );
                    rec->dormant = false;


                    g_interfaces.engine_client->get_player_info( player->index( ), &info.player_info );
                    update_player_animation( player, *rec, previous );
                    const auto backup_lean = player->anim_overlays( )[ 12 ].weight;


                    //std::memcpy( player->anim_overlays( ), rec->layer_records.data( ), sizeof( rec->layer_records ) );
                    //std::memcpy( player->pose_parameters( ).data( ), rec->pose_parameters.data( ), sizeof( rec->pose_parameters ) );

                    player->anim_overlays( )[ 12 ].weight = 0.0f;
                    g_bone_setup.build( player, rec->bones.data( ), bone_used_by_anything, rec->origin, player->get_abs_angles( ), rec->sim_time, player->eye_angles( ) );
                    player->anim_overlays( )[ 12 ].weight = backup_lean;
   
                    info.lag_records.push_front( &info.anim_records.front( ) );
                }
            }
        }

        const int dead_time = game::ticks_to_time( globals::local_player->tick_base( ) ) - globals::cvars::sv_maxunlag->get_float( );
        auto tail_index = info.lag_records.size( ) - 1;

        while ( tail_index < info.lag_records.size( ) ) {
            const auto tail = info.lag_records[ tail_index ];

            if ( tail->sim_time >= dead_time )
                break;

            info.lag_records.pop_back( );
            tail_index = info.lag_records.size( ) - 1;
        }
    }
}

void animation_sync::apply_animations( ) {
    if ( !g_interfaces.engine_client->is_in_game( ) || !g_interfaces.engine_client->is_connected( ) )
        return;

    if ( !globals::local_player )
        return;

    for ( int i = 1; i < g_interfaces.global_vars->max_clients; ++i ) {
        const auto player = g_interfaces.entity_list->get_client_entity< c_cs_player * >( i );

        if ( !player || player->dormant( ) || !player->alive( ) || !player->is_player( ) || player == globals::local_player )
            continue;

        auto &info = player_log[ player->index( ) ];

        if ( info.anim_records.empty( ) )
            continue;

        auto &front = info.anim_records.front( );

        animated_bones[ player->index( ) ] = front.bones;
        animated_origin[ player->index( ) ] = front.origin;

        player->pose_parameters( ) = front.pose_parameters;
        player->set_abs_angles( front.abs_angles );
    }
}