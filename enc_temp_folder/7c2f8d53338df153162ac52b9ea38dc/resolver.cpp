#include "resolver.hpp"
#include <features/animations/animation_state.hpp>
#include <threadutils/threading.h>

void resolver::on_proxy_update( c_cs_player *player, float updated_value ) {
    auto player_log = &g_animations.player_log[ player->index( ) ];

    if ( !player_log )
        return;

    if ( player_log->body != updated_value )
        player_log->body = updated_value;
}

bool resolver::anti_freestanding( lag_record &record ) {
    auto player_log = &g_animations.player_log[ record.player->index( ) ];

    if ( !player_log )
        return false;

    if ( player_log->anti_freestand_record.left_damage >= 20.0f && player_log->anti_freestand_record.right_damage >= 20.0f )
        return false;

    const float at_target_yaw = math::vector_angle( record.player->origin( ) - globals::local_player->origin( ) ).y;

    if ( player_log->anti_freestand_record.left_damage <= 0.0f && player_log->anti_freestand_record.right_damage <= 0.0f ) {
        if ( player_log->anti_freestand_record.right_fraction < player_log->anti_freestand_record.left_fraction )
            record.eye_angles.y = at_target_yaw + 125.f;
        else
            record.eye_angles.y = at_target_yaw - 73.f;
    } else {
        if ( player_log->anti_freestand_record.left_damage > player_log->anti_freestand_record.right_damage )
            record.eye_angles.y = at_target_yaw + 130.f;
        else
            record.eye_angles.y = at_target_yaw - 49.f;
    }

    return true;
}

std::pair< float, bool > resolver::AntiFreestand( c_cs_player *player, lag_record *record, bool include_base, float base_yaw, float delta ) {
    constexpr float STEP{ 4.f };
    constexpr float RANGE{ 32.f };

    std::vector< adaptive_angle > angles;

    angles.emplace_back( base_yaw + delta );
    angles.emplace_back( base_yaw - delta );

    if ( include_base )
        angles.emplace_back( base_yaw );

    auto start = globals::local_player->get_shoot_position( );
    bool valid{ false };
    auto shoot_pos = player->get_shoot_position( );

    for ( auto it = angles.begin( ); it != angles.end( ); ++it ) {
        vector_3d end{ shoot_pos.x + std::cos( math::deg_to_rad( it->yaw ) ) * RANGE,
                       shoot_pos.y + std::sin( math::deg_to_rad( it->yaw ) ) * RANGE,
                       shoot_pos.z };

        vector_3d dir = end - start;
        float len = math::normalize_place( dir );

        if ( len <= 0.f )
            continue;

        for ( float i{ 0.f }; i < len; i += STEP ) {
            vector_3d point = start + ( dir * i );
            int contents = g_interfaces.engine_trace->get_point_contents( point, mask_shot_hull );

            if ( !( contents & mask_shot_hull ) )
                continue;

            float mult = 1.f;

            if ( i > ( len * 0.5f ) )
                mult = 1.25f;

            if ( i > ( len * 0.75f ) )
                mult = 1.25f;

            if ( i > ( len * 0.9f ) )
                mult = 2.f;

            it->dist += ( STEP * mult );
            valid = true;
        }
    }

    if ( !valid )
        return { base_yaw, false };

    std::sort( angles.begin( ), angles.end( ),
               []( const adaptive_angle &a, const adaptive_angle &b ) {
                   return a.dist > b.dist;
               } );

    return { angles.front( ).dist, true };
}
void resolver::start( c_cs_player *player, lag_record &record, lag_record *previous ) {
    auto player_log = &g_animations.player_log[ player->index( ) ];

    if ( !player_log )
        return;

    if ( player_log->player_info.fake_player ) {
        record.mode = resolve_mode::none;
        return;
    }

    player_log->body = record.lower_body_yaw;

    const auto moving = glm::length( record.velocity ) > 0.1f && !record.fake_walk;
    const auto diff = math::normalize_angle( player_log->body - player_log->walk_record.lower_body_yaw );
    const auto layer_activity = player->get_sequence_activity( record.layer_records[ ANIMATION_LAYER_ADJUST ].sequence );
    const auto velyaw = math::rad_to_deg( std::atan2( record.velocity.y, record.velocity.x ) );
    const auto away_target = math::normalize( math::angle_from_vectors( globals::local_player->origin( ), player->origin( ) ).y );
    const auto in_air = !record.on_ground;

    player_log->anti_freestand_record.left_damage = 0.0f;
    player_log->anti_freestand_record.right_damage = 0.0f;
    player_log->anti_freestand_record.left_fraction = 0.0f;
    player_log->anti_freestand_record.right_fraction = 0.0f;

    if ( moving && record.on_ground )
        std::memcpy( &player_log->walk_record, &record, sizeof( lag_record ) );

    if ( record.sim_time > 0.f ) {
        const auto delta = player_log->walk_record.origin - record.origin;

        if ( glm::length( delta ) <= 128.f ) {
            record.moved = true;
        }
    }

    vector_3d direction_1, direction_2;

    math::angle_vectors( vector_3d( 0.f, math::angle_vectors( record.origin - globals::local_player->origin( ) ).y - 90.f, 0.f ), &direction_1 );
    math::angle_vectors( vector_3d( 0.f, math::angle_vectors( record.origin - globals::local_player->origin( ) ).y + 90.f, 0.f ), &direction_2 );

    const auto left_eye_pos = record.origin + vector_3d( 0.f, 0.f, 64.f ) + ( direction_1 * 16.f );
    const auto right_eye_pos = record.origin + vector_3d( 0.f, 0.f, 64.f ) + ( direction_2 * 16.f );

    player_log->anti_freestand_record.left_damage = g_penetration.run( globals::local_player->get_shoot_position( ), left_eye_pos, player, record.bones, false ).out_damage;
    player_log->anti_freestand_record.right_damage = g_penetration.run( globals::local_player->get_shoot_position( ), right_eye_pos, player, record.bones, false ).out_damage;

    ray_t ray;
    c_game_trace trace;
    c_trace_filter_hitscan filter;

    filter.player = globals::local_player;

    ray.init( left_eye_pos, globals::local_player->get_shoot_position( ) );
    g_interfaces.engine_trace->trace_ray( ray, mask_all, &filter, &trace );
    player_log->anti_freestand_record.left_fraction = trace.fraction;

    ray.init( right_eye_pos, globals::local_player->get_shoot_position( ) );
    g_interfaces.engine_trace->trace_ray( ray, mask_all, &filter, &trace );
    player_log->anti_freestand_record.right_fraction = trace.fraction;

    player_log->resolve_record.predicted_lby_flick = false;
    player_log->resolve_record.lby_flick = false;
    player_log->lby_updated = false;

    const auto delta_time = record.anim_time - player_log->walk_record.anim_time;

    //bool is_jittering_fake = false;
    //bool is_static_real = false;

    //if ( player_log->lag_records.size( ) > 2 ) {
    //    auto &latest_record = player_log->lag_records[ player_log->lag_records.size( ) - 1 ];
    //    auto &prev_record = player_log->lag_records[ player_log->lag_records.size( ) - 2 ];

    //    if ( abs( latest_record->eye_angles.y - prev_record->eye_angles.y ) > 35.0f ) {
    //        is_jittering_fake = true;
    //    }

    //    if ( abs( latest_record->lower_body_yaw - prev_record->lower_body_yaw ) < 1.0f ) {
    //        is_static_real = true;
    //    }
    //}

    if ( !record.on_ground ) {
        record.mode = resolve_mode::air;
        player->eye_angles( ).y = velyaw;

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
        }

        return;
    }

    if ( record.moved ) {
        if ( record.anim_time >= player_log->body_update_time ) {
            record.mode = resolve_mode::lby_update;

            player_log->body_update_time = record.sim_time + 1.1f;
            player_log->lby_updated = true;
            player->eye_angles( ).y = record.lower_body_yaw;

            return;
        }

        //else if ( player->lower_body_yaw_target( ) != player_log->last_lby ) {
        //    player->eye_angles( ).y = player_log->body;
        //    record.mode = resolve_mode::lby_update;

        //    player_log->lby_updated = true;
        //    player_log->last_lby = player->lower_body_yaw_target( );

        //    return;
        //}

        if ( player_log->missed_shots >= 1 ) {
            record.mode = resolve_mode::brute;
            switch ( player_log->missed_shots % 5 ) {
                case 1:
                    player->eye_angles( ).y = away_target;
                    break;
                case 2:
                    player->eye_angles( ).y = velyaw - 90.0f;
                    break;
                case 3:
                    player->eye_angles( ).y = away_target + 180.0f;
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
            if ( anti_freestanding( record ) ) {
                record.mode = resolve_mode::freestand;
                return;
            }
        }
    } else if ( moving && !record.on_ground ) {
        record.mode = resolve_mode::moving;
        player_log->body_update_time = record.sim_time;
        return;
    } else if ( moving && record.on_ground ) {
        player_log->body_update_time = record.sim_time + 0.22f;
        player->eye_angles( ).y = player_log->body;
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

        if ( it->mode == resolve_mode::none || it->mode == resolve_mode::moving || it->mode == resolve_mode::standing || it->mode == resolve_mode::freestand || it->mode == resolve_mode::lby_update )
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