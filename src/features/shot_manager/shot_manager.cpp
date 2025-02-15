#include "shot_manager.hpp"
#include <features/ragebot/ragebot.hpp>
#include <features/ui/notifications/notifications.hpp>
#include <features/visuals/visuals.hpp>

void shot_manager::on_shot_fire( c_cs_player *target, float damage, int bullets, lag_record *record ) {
    auto &log = g_animations.player_log[ target->index( ) ];

    for ( int i{ }; i < bullets; ++i ) {
        shot_record_t shot;

        shot.target = target;
        shot.record = record;
        shot.time = g_interfaces.global_vars->curtime;
        shot.lag = record->choked;
        shot.lat = globals::latency;
        shot.damage = damage;
        shot.impacted = false;
        shot.hurt_player = false;
        shot.pos = globals::local_player->get_shoot_position( );

        if ( target && i == 0 )
            ++log.shots;

        shots.push_front( shot );
    }

    while ( shots.size( ) > 128 )
        shots.pop_back( );
}

void shot_manager::reset_data( ) {
    if ( !globals::local_player )
        return;

    for ( int i = 1; i < g_interfaces.global_vars->max_clients; i++ ) {
        auto player = g_interfaces.entity_list->get_client_entity< c_cs_player * >( i );

        if ( !player || !player->is_player( ) || player == globals::local_player )
            continue;

        auto player_log = &g_animations.player_log[ player->index( ) ];

        if ( !player_log )
            continue;

        player_log->shots = 0;
        player_log->missed_shots = 0;
    }

    shots.clear( );
    vis_impacts.clear( );
    hits.clear( );
    impacts.clear( );
}

void shot_manager::on_round_start( event_t *evt ) {
    if ( !evt || !globals::local_player )
        return;

    reset_data( );
}

void shot_manager::process_shots( ) {
    if ( !globals::local_player || !globals::local_player->alive( ) ) {
        reset_data( );
        return;
    }

    if ( shots.empty( ) ) return;

    auto &shot = shots.front( );

    auto target = shot.target;

    if ( !target )
        return;

    auto player_log = &g_animations.player_log[ target->index( ) ];

    if ( !player_log )
        return;

    if ( !shot.record )
        return;

    if ( !target->collideable( ) )
        return;

    float predicted = shot.time;
    float delta = std::fabs( g_interfaces.global_vars->curtime - predicted );

    if ( shot.matched && shot.impacted && shot.time != -1.0f ) {
        const auto backup_origin = target->origin( );
        const auto backup_mins = target->collideable( )->mins( );
        const auto backup_maxs = target->collideable( )->maxs( );
        const auto backup_bones = target->bone_cache( );

        shot.record->cache( );

        if ( !shot.hurt_player ) {
            vector_3d pos, dir, start, end;
            c_game_trace trace;

            start = shot.pos;
            dir = math::normalize_angle( shot.server_pos - start );
            end = start + ( dir * 8192.f );

            const auto had_prediction_error = globals::local_player->velocity_modifier( ) < 1.0f && g_interfaces.prediction->prev_ack_had_errors > 0;
            const auto should_hit_player = g_ragebot.can_hit( target, start, end, shot.record, shot.record->bones.data( ) );

            const auto get_missed_shot = [ & ]( ) -> const char * {
                const char *missed_shot = _xs( "spread" );

                if ( player_log->player_info.fake_player )
                    return missed_shot;

                else {
                    if ( had_prediction_error )
                        missed_shot = _xs( "prediction error" );

                    else if ( !target->alive( ) )
                        missed_shot = _xs( "enemy death" );

                    else if ( !globals::local_player->alive( ) )
                        missed_shot = _xs( "local death" );

                    if ( should_hit_player ) {
                        missed_shot = _xs( "resolver" );

                        ++player_log->missed_shots;
                    }
                }

                return missed_shot;
            };

            g_notify.add( notify_type::none, false, fmt::format( _xs( "Missed shot due to {}" ), get_missed_shot( ) ) );

            if ( !player_log->player_info.fake_player )
                g_notify.add( notify_type::miss, true, fmt::format( _xs( "reason: {} | mode: {} | target: {} | hitgroup : {} | dmg : {} | lc : {} | lag: {}" ), get_missed_shot( ), static_cast< int >( shot.record->mode ), player_log->player_info.name, hitgroup_names[ shot.hurt_player ], shot.damage, shot.record->break_lc ? _xs( "false" ) : _xs( "true" ), shot.record->choked ) );
        }

        target->origin( ) = backup_origin;
        target->set_collision_bounds( backup_mins, backup_maxs );
        target->bone_cache( ) = backup_bones;

        shots.erase( shots.begin( ) );
    }
}

void shot_manager::on_fire( event_t *evt ) {
    if ( !evt || !globals::local_player )
        return;

    auto attacker = g_interfaces.engine_client->get_player_for_user_id( evt->get_int( _xs( "userid" ) ) );

    if ( attacker != g_interfaces.engine_client->get_local_player( ) )
        return;

    if ( shots.empty( ) )
        return;

    shot_record_t *shot = &shots.front( );

    if ( !shot )
        return;

    shot->matched = true;
}

void shot_manager::on_impact( event_t *evt ) {
    int attacker;
    vector_3d pos, dir, start, end;
    float time;

    if ( !evt || !globals::local_player )
        return;

    attacker = g_interfaces.engine_client->get_player_for_user_id( evt->get_int( _xs( "userid" ) ) );

    if ( attacker != g_interfaces.engine_client->get_local_player( ) )
        return;

    pos = {
            evt->get_float( _xs( "x" ) ),
            evt->get_float( _xs( "y" ) ),
            evt->get_float( _xs( "z" ) ) };

    time = g_interfaces.global_vars->curtime;

    if ( g_vars.visuals_other_bullet_impacts.value )
        g_interfaces.debug_overlay->add_box_overlay( pos, vector_3d( -2.0f, -2.0f, -2.0f ), vector_3d( 2.0f, 2.0f, 2.0f ), vector_3d( 0.0f, 0.0f, 0.0f ), g_vars.visuals_other_server_bullet_impact_col.value.r, g_vars.visuals_other_server_bullet_impact_col.value.g, g_vars.visuals_other_server_bullet_impact_col.value.b, g_vars.visuals_other_server_bullet_impact_col.value.a, 7.0f );

    if ( shots.empty( ) )
        return;

    shot_record_t *shot = &shots.front( );

    if ( !shot )
        return;

    shot->impacted = true;

    impact_record_t impact;

    impact.shot = shot;
    impact.tick = globals::local_player->tick_base( );
    impact.pos = pos;

    if ( shot->matched )
        shot->server_pos = pos;

    impacts.push_front( impact );

    while ( impacts.size( ) > 128 )
        impacts.pop_back( );
}

void shot_manager::on_hurt( event_t *evt ) {
    int attacker, victim, group, hp;
    float damage;
    float time;
    std::string name;

    if ( !evt || !globals::local_player )
        return;

    attacker = g_interfaces.engine_client->get_player_for_user_id( evt->get_int( _xs( "attacker" ) ) );
    victim = g_interfaces.engine_client->get_player_for_user_id( evt->get_int( _xs( "userid" ) ) );

    if ( attacker < 1 || attacker > 64 || victim < 1 || victim > 64 )
        return;

    else if ( attacker != g_interfaces.engine_client->get_local_player( ) || victim == g_interfaces.engine_client->get_local_player( ) )
        return;

    group = evt->get_int( _xs( "hitgroup" ) );

    if ( group == hitgroups::hitgroup_gear )
        return;

    auto target = g_interfaces.entity_list->get_client_entity< c_cs_player * >( victim );

    if ( !target )
        return;

    player_info_t info;

    if ( !g_interfaces.engine_client->get_player_info( victim, &info ) )
        return;

    name = std::string( info.name ).substr( 0, 24 );
    damage = static_cast< float >( evt->get_int( _xs( "dmg_health" ) ) );
    hp = evt->get_int( _xs( "health" ) );

    if ( g_vars.misc_hitmarker.value ) {
        g_visuals.hitmarker_fraction = 1.0f;
        g_interfaces.engine_client->client_cmd_unrestricted( _xs( "play buttons/arena_switch_press_02.wav" ) );
    }

    if ( g_vars.misc_events_log_damage.value )
        g_notify.add( notify_type::none, false, fmt::format( _xs( "Hit {} in the {} for {} ({} health remaining)" ), name, hitgroup_names[ group ], ( int ) damage, hp ) );

    time = g_interfaces.global_vars->curtime;

    shot_record_t *matched_shot = nullptr;
    auto best_delta = std::numeric_limits< float >::max( );

    if ( shots.empty( ) )
        return;

    auto &shot = shots.front( );

    if ( shot.hurt_player )
        return;

    float predicted = shot.time;
    float delta = std::fabs( time - predicted );

    if ( delta > 1.0f ) {
        shots.erase( shots.begin( ) );
        return;
    }

    shot.hurt_player = true;

    if ( group == hitgroup_generic )
        return;

    if ( !vis_impacts.empty( ) ) {
        for ( auto &i : vis_impacts ) {
            if ( i.tickbase == globals::local_player->tick_base( ) )
                i.hurt_player = true;
        }
    }

    if ( impacts.empty( ) )
        return;

    impact_record_t *impact{ nullptr };

    for ( auto &i : impacts ) {
        if ( i.tick != globals::local_player->tick_base( ) )
            continue;

        if ( i.shot->target != target )
            continue;
        impact = &i;
        break;
    }

    if ( !impact )
        return;

    hit_record_t hit;
    hit.impact = impact;
    hit.group = group;
    hit.damage = damage;

    hits.push_front( hit );

    while ( hits.size( ) > 128 )
        hits.pop_back( );
}