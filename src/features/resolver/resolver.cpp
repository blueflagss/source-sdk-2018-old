#include "resolver.hpp"
#include <features/animations/animation_state.hpp>
#include <threadutils/threading.h>

void resolver::on_proxy_update( c_cs_player *player, float updated_value ) {
    auto player_log = &g_animations.player_log[ player->index( ) ];

    if ( !player_log )
        return;

    //if ( player_log->body != updated_value )
    //   player_log->body = updated_value;
}

bool resolver::anti_freestanding( lag_record &record ) {
    auto player_log = &g_animations.player_log[ record.player->index( ) ];

    if ( !player_log )
        return false;

    if ( player_log->direction_info.left_damage >= 20.0f && player_log->direction_info.right_damage >= 20.0f )
        return false;

    const float at_target_yaw = math::vector_angle( record.player->origin( ) - globals::local_player->origin( ) ).y;

    if ( player_log->direction_info.left_damage <= 0.0f && player_log->direction_info.right_damage <= 0.0f ) {
        if ( player_log->direction_info.right_fraction < player_log->direction_info.left_fraction )
            record.player->eye_angles( ).y = at_target_yaw + 90.f;
        else
            record.player->eye_angles( ).y = at_target_yaw - 90.f;
    } else {
        if ( player_log->direction_info.left_damage > player_log->direction_info.right_damage )
            record.player->eye_angles( ).y = at_target_yaw + 90.f;
        else
            record.player->eye_angles( ).y = at_target_yaw - 90.f;
    }

    return true;
}

void resolver::start( c_cs_player *player, lag_record &record, lag_record *previous ) {
    auto player_log = &g_animations.player_log[ player->index( ) ];

    const auto anim_state = player->anim_state( );

    if ( !player_log || !anim_state || player_log->player_info.fake_player ) {
        record.mode = resolve_mode::none;
        return;
    }

    const auto moving = glm::length( record.velocity ) > 0.1f && !record.fake_walk;
    const auto layer_activity = player->get_sequence_activity( record.layer_records[ ANIMATION_LAYER_ADJUST ].sequence );
    const auto velyaw = math::rad_to_deg( std::atan2( record.velocity.y, record.velocity.x ) );
    const auto away_target = math::normalize( math::angle_from_vectors( globals::local_player->origin( ), player->origin( ) ).y );
    const auto in_air = !record.on_ground;

    player_log->direction_info.left_damage = 0.0f;
    player_log->direction_info.right_damage = 0.0f;
    player_log->direction_info.left_fraction = 0.0f;
    player_log->direction_info.right_fraction = 0.0f;

    if ( !moving ) {
        vector_3d direction_1, direction_2;

        math::angle_vectors( vector_3d( 0.f, math::normalize( math::angle_vectors( record.origin - globals::local_player->origin( ) ).y ) - 90.f, 0.f ), &direction_1 );
        math::angle_vectors( vector_3d( 0.f, math::normalize( math::angle_vectors( record.origin - globals::local_player->origin( ) ).y ) + 90.f, 0.f ), &direction_2 );

        const auto left_eye_pos = record.origin + vector_3d( 0.f, 0.f, 64.f ) + ( direction_1 * 16.f );
        const auto right_eye_pos = record.origin + vector_3d( 0.f, 0.f, 64.f ) + ( direction_2 * 16.f );

        player_log->direction_info.left_damage = g_penetration.run( globals::local_player->get_shoot_position( ), left_eye_pos, player, 0.0f, record.bones, false ).out_damage;
        player_log->direction_info.right_damage = g_penetration.run( globals::local_player->get_shoot_position( ), right_eye_pos, player, 0.0f, record.bones, false ).out_damage;

        ray_t ray;
        c_game_trace trace;
        c_trace_filter_hitscan filter;

        filter.player = globals::local_player;

        ray.init( left_eye_pos, globals::local_player->get_shoot_position( ) );
        g_interfaces.engine_trace->trace_ray( ray, mask_all, &filter, &trace );
        player_log->direction_info.left_fraction = trace.fraction;

        ray.init( right_eye_pos, globals::local_player->get_shoot_position( ) );
        g_interfaces.engine_trace->trace_ray( ray, mask_all, &filter, &trace );
        player_log->direction_info.right_fraction = trace.fraction;

        player_log->resolve_record.predicted_lby_flick = false;
        player_log->resolve_record.lby_flick = false;
        player_log->lby_updated = false;
    }

    if ( !record.on_ground ) {
        if ( player_log->missed_shots > 0 ) {
            record.mode = resolve_mode::air_brute;

            switch ( player_log->missed_shots % 3 ) {
                case 1:
                    player->eye_angles( ).y = velyaw + 180.f;
                    break;
                case 2:
                    player->eye_angles( ).y = velyaw - 90.f;
                    break;
                case 3:
                    player->eye_angles( ).y = velyaw + 90.f;
                    break;
            }
        } else {
            record.mode = resolve_mode::air;
            player->eye_angles( ).y = velyaw;
        }

        return;
    }

    if ( moving ) {
        record.mode = resolve_mode::moving;
        player_log->last_lby = player->lower_body_yaw_target( );
        player->eye_angles( ).y = player->lower_body_yaw_target( );

        player_log->body_update_time = record.sim_time + 0.22f;
        player_log->lby_updated = false;
        return;
    } else {
        if ( player_log->missed_shots > 0 ) {
            record.mode = resolve_mode::brute;
            switch ( player_log->missed_shots % 5 ) {
                case 1:
                    player->eye_angles( ).y = velyaw;
                    break;
                case 2:
                    player->eye_angles( ).y = away_target;
                    break;
                case 3:
                    player->eye_angles( ).y = away_target - 90.0f;
                    break;
                case 4:
                    player->eye_angles( ).y = record.fake_walk ? player_log->body - 110.f : velyaw + 180.0f;
                    break;
                case 5:
                    player->eye_angles( ).y = away_target;
                    break;
                default:
                    break;
            }
        } else {
            record.mode = resolve_mode::standing;
            anti_freestanding( record );

            if ( previous && valve_math::angle_diff( record.lower_body_yaw, previous->lower_body_yaw ) > 1.0f ) {
                record.mode = resolve_mode::lby_update;

                player->eye_angles( ).y = player_log->last_lby;
                player_log->body_update_time = record.sim_time + 1.1f;
                player_log->lby_updated = true;
                return;
            }

            if ( record.sim_time >= player_log->body_update_time ) {
                record.mode = resolve_mode::lby_update;

                player->eye_angles( ).y = player_log->last_lby;
                player_log->body_update_time = record.sim_time + 1.1f;
                player_log->lby_updated = true;

                return;
            }
        }
    }
}

lag_record *resolver::find_ideal_record( c_cs_player *player ) {
    if ( !player )
        return nullptr;

    auto weapon = g_interfaces.entity_list->get_client_entity_from_handle< c_cs_weapon_base * >( player->weapon_handle( ) );

    if ( !weapon )
        return nullptr;

    auto &player_log = g_animations.player_log[ player->index( ) ];

    lag_record *first_valid = nullptr;
    lag_record *current = nullptr;

    if ( player_log.lag_records.empty( ) )
        return nullptr;

    for ( auto &it : player_log.lag_records ) {
        if ( !it || it->dormant || !it->is_valid( ) )
            continue;

        current = it;

        if ( !first_valid )
            first_valid = current;

        if ( it->mode >= resolve_mode::none || it->mode == resolve_mode::lby_update )
            return current;
    }

    return ( first_valid ) ? first_valid : find_last_record( player );
}

lag_record *resolver::find_last_record( c_cs_player *player ) {
    auto &log = g_animations.player_log[ player->index( ) ];

    if ( !log.player )
        return nullptr;

    lag_record *current = nullptr;

    if ( log.lag_records.empty( ) )
        return nullptr;

    for ( int i = static_cast< int >( log.lag_records.size( ) ) - 1; i >= 0; i-- ) {
        auto &record = log.lag_records[ i ];

        if ( !record )
            continue;

        current = record;

        if ( current->is_valid( ) )
            return current;
    }

    return nullptr;
}