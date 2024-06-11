#include "aimbot.hpp"

#include <core/config.hpp>
#include <features/engine_prediction/engine_prediction.hpp>
#include <features/resolver/resolver.hpp>
#include <features/ui/notifications/notifications.hpp>
#include <threadutils/threading.h>
#include <utils/threading/dispatch.hpp>

void aimbot::reset( ) {
    best = { };
    globals::target_index = -1;
    globals::is_targetting = false;
    targets.clear( );
    aim_points.clear( );
}

bool aimbot::hitchance( c_cs_player *player, const vector_3d &angle, lag_record *record ) {
    auto weapon_data = globals::local_weapon->get_weapon_data( );

    if ( !weapon_data )
        return false;

    size_t total_hits{ };

    const auto needed_hits{ static_cast< size_t >( ( g_vars.aimbot_hit_chance.value * 255 / 100.f ) ) };
    const auto inaccuracy = globals::local_weapon->get_inaccuracy( );
    const auto spread = globals::local_weapon->get_spread( );

    auto *studio_model = g_interfaces.model_info->get_studio_model( player->get_model( ) );

    if ( !studio_model )
        return false;

    auto *hitbox_set = studio_model->hitbox_set( player->hitbox_set( ) );
    vector_3d mins{ }, maxs{ };

    auto *hitbox = hitbox_set->hitbox( best.hitbox );

    if ( ( g_prediction.ideal_inaccuracy + 0.0005f ) >= inaccuracy ) {
        return true;
    }

    matrix_3x4 bone_transform;
    memcpy( &bone_transform, &record->bones[ hitbox->bone ], sizeof( matrix_3x4 ) );

    if ( hitbox->angle != vector_3d( ) ) {
        matrix_3x4 temp;

        math::angle_matrix( hitbox->angle, temp );
        math::concat_transforms( bone_transform, temp, bone_transform );
    }

    vector_3d vMin, vMax;
    math::vector_transform( hitbox->min, bone_transform, vMin );
    math::vector_transform( hitbox->max, bone_transform, vMax );

    vector_3d forward{ }, right{ }, up{ };

    math::angle_vectors( angle, &forward, &right, &up );

    for ( auto i = 0; i <= 255; i++ ) {
        const auto weapon_spread = globals::local_weapon->calculate_spread( i, inaccuracy, spread );

        auto dir = math::normalize_angle( forward + ( right * weapon_spread.x ) + ( up * weapon_spread.y ) );

        const auto end = globals::shoot_position + ( dir * weapon_data->range );

        if ( hitbox->group <= 0.f ) {
            c_game_trace trace;
            ray_t ray;
            ray.init( globals::shoot_position, end );

            g_interfaces.engine_trace->clip_ray_to_entity( ray, mask_shot_hull | contents_hitbox, player, &trace );

            if ( trace.entity == player && trace.hitbox == best.hitbox )
                ++total_hits;
        } else {
            float m1, m2;
            float dist = math::dist_segment_to_segment_sqr( globals::shoot_position, end, vMin, vMax, m1, m2 );

            if ( dist <= hitbox->radius * hitbox->radius ) {
                total_hits++;
            }
        }

        if ( total_hits >= needed_hits )
            return true;

        if ( ( ( 255 - i ) + total_hits ) < needed_hits )
            return false;
    }

    return false;
}

bool aimbot::extrapolate_target( aim_player &target ) {
    auto &records = g_animations.lag_info[ target.entity->index( ) ].anim_records;

    if ( records.empty( ) )
        return false;

    if ( target.entity->dormant( ) )
        return false;

    auto state = target.entity->anim_state( );

    if ( !state )
        return false;

    size_t size{ };

    for ( const auto &it : records ) {
        if ( it.dormant )
            break;

        ++size;
    }

    auto record = records[ 0 ];

    if ( !record.break_lc )
        return false;

    int simulation = game::time_to_ticks( record.sim_time );

    if ( std::abs( globals::arrival_tick - simulation ) >= 128 )
        return true;

    auto lag = std::clamp< int >( record.choked - 1, 0, 17 );

    int updatedelta = globals::server_tick - record.tick;

    if ( globals::latency_ticks <= lag - updatedelta )
        return true;

    int next = record.tick + 1;

    if ( next + lag >= globals::arrival_tick )
        return true;

    int pred = 0;

    while ( true ) {
        for ( int i = 0; i < lag; i++ )
            g_animations.extrapolate( target.entity, target.entity->origin( ), target.entity->velocity( ), target.entity->flags( ), ( target.entity->flags( ) & player_flags::on_ground ) || state->m_bOnGround );

        pred++;
    }

    if ( pred <= 0 )
        return true;
    
    g_animations.update_player_animation( target.entity, *target.record, nullptr, false );

    return false;
}

bool aimbot::get_hitbox_position( c_cs_player *player, matrix_3x4 *bones, int i, vector_3d &position ) {
    auto model = player->get_model( );

    if ( !model )
        return false;

    auto studio_hdr = g_interfaces.model_info->get_studio_model( model );

    if ( !studio_hdr )
        return false;

    auto hitbox_set = studio_hdr->hitbox_set( player->hitbox_set( ) );

    if ( !hitbox_set )
        return false;

    auto hitbox = hitbox_set->hitbox( static_cast< int >( i ) );

    if ( !hitbox )
        return false;

    vector_3d min, max;

    math::vector_transform( hitbox->min, bones[ hitbox->bone ], min );
    math::vector_transform( hitbox->max, bones[ hitbox->bone ], max );

    position = ( min + max ) * 0.5f;

    return true;
};

void aimbot::search_targets( ) {
    for ( int n = 1; n <= ( g_interfaces.entity_list->get_highest_entity_index( ) + 1 ); ++n ) {
        const auto entity = g_interfaces.entity_list->get_client_entity< c_base_player * >( n );

        if ( !entity || entity->dormant( ) || entity->team( ) == globals::local_player->team( ) || entity == globals::local_player )
            continue;

        switch ( HASH( entity->get_client_class( )->network_name ) ) {
            case HASH_CT( "CCSPlayer" ): {
                auto player = entity->get< c_cs_player * >( );

                if ( !player || !player->alive( ) )
                    break;

                targets.emplace_back( aim_player{ player, math::calculate_fov( globals::view_angles, math::clamp_angle( player->get_shoot_position( ) ) ), glm::length( player->origin( ) - globals::local_player->origin( ) ), 0, &g_animations.lag_info[ player->index( ) ].anim_records.front( ), vector_3d( 0, 0, 0 ), vector_3d( 0, 0, 0 ) } );
            } break;
        }
    }

    if ( targets.empty( ) )
        return;

    std::sort( targets.begin( ), targets.end( ), [ & ]( aim_player &lhs, aim_player &rhs ) {
        if ( fabsf( lhs.fov - rhs.fov ) < 20.0f )
            return lhs.entity->health( ) < rhs.entity->health( );

        switch ( g_vars.aimbot_sort_by.value ) {
            case 0: {
                return lhs.distance < rhs.distance;
            } break;
            case 1: {
                return lhs.fov < rhs.fov;
            } break;
        }
    } );
}

void aimbot::generate_points_for_hitbox( c_cs_player *player, lag_record *record, int side, std::vector< std::pair< vector_3d, bool > > &points, mstudiobbox_t *hitbox, mstudiohitboxset_t *set, int idx, float scale ) {
    vector_3d center = ( hitbox->max + hitbox->min ) * 0.5f;

    vector_3d center_transformed;
    math::vector_transform( center, record->bones[ hitbox->bone ], center_transformed );

    points.push_back( std::make_pair( center_transformed, false ) );

    if ( scale <= 0.0f )
        return;

    if ( hitbox->radius <= 0.0f ) {
        if ( idx == csgo_hitbox::hitbox_r_foot || idx == csgo_hitbox::hitbox_l_foot ) {
            float d1 = ( hitbox->min.z - center.z ) * 0.425f;

            if ( idx == csgo_hitbox::hitbox_l_foot )
                d1 *= -1.f;

            vector_3d toe = vector_3d{ ( ( hitbox->max.x - center.x ) * scale ) + center.x, center.y, center.z };
            vector_3d heel = vector_3d{ ( ( hitbox->min.x - center.x ) * scale ) + center.x, center.y, center.z };

            points.push_back( std::make_pair( math::vector_transform( toe, record->bones[ hitbox->bone ] ), true ) );
            points.push_back( std::make_pair( math::vector_transform( heel, record->bones[ hitbox->bone ] ), true ) );
        }
    } else {
        float r = hitbox->radius * scale;

        if ( idx == csgo_hitbox::hitbox_head ) {
            vector_3d left{ hitbox->max.x, hitbox->max.y, hitbox->max.z - ( hitbox->radius * 0.5f ) };
            points.push_back( std::make_pair( math::vector_transform( left, record->bones[ hitbox->bone ] ), true ) );

            vector_3d right{ hitbox->max.x, hitbox->max.y, hitbox->max.z + ( hitbox->radius * 0.5f ) };
            points.push_back( std::make_pair( math::vector_transform( right, record->bones[ hitbox->bone ] ), true ) );

            constexpr float rotation = 0.70710678f;

            scale = std::clamp< float >( scale, 0.1f, 0.95f );
            r = hitbox->radius * scale;

            vector_3d topback{ hitbox->max.x + ( rotation * r ), hitbox->max.y + ( -rotation * r ), hitbox->max.z };
            points.push_back( std::make_pair( math::vector_transform( topback, record->bones[ hitbox->bone ] ), true ) );
            points.push_back( std::make_pair( math::vector_transform( vector_3d( center.x, hitbox->max.y - r, center.z ), record->bones[ hitbox->bone ] ), true ) );
        } else if ( idx == csgo_hitbox::hitbox_body || idx == csgo_hitbox::hitbox_pelvis ) {
            vector_3d back{ center.x, hitbox->max.y - r, center.z };
            vector_3d right{ hitbox->max.x, hitbox->max.y, hitbox->max.z + ( hitbox->radius * 0.5f ) };
            vector_3d left{ hitbox->max.x, hitbox->max.y, hitbox->max.z - ( hitbox->radius * 0.5f ) };

            points.push_back( std::make_pair( math::vector_transform( back, record->bones[ hitbox->bone ] ), true ) );
            points.push_back( std::make_pair( math::vector_transform( right, record->bones[ hitbox->bone ] ), true ) );
            points.push_back( std::make_pair( math::vector_transform( left, record->bones[ hitbox->bone ] ), true ) );
        } else if ( idx == csgo_hitbox::hitbox_thorax || idx == csgo_hitbox::hitbox_chest || idx == csgo_hitbox::hitbox_upper_chest ) {
            vector_3d back{ center.x, hitbox->max.y - r, center.z };
            points.push_back( std::make_pair( math::vector_transform( back, record->bones[ hitbox->bone ] ), true ) );
        } else if ( idx == csgo_hitbox::hitbox_r_thigh || idx == csgo_hitbox::hitbox_l_thigh ) {
            vector_3d half_bottom{ hitbox->max.x - ( hitbox->radius * 0.5f ), hitbox->max.y, hitbox->max.z };
            points.push_back( std::make_pair( math::vector_transform( half_bottom, record->bones[ hitbox->bone ] ), true ) );
        }
    }
}

static void run_hitscan( void *arg ) {
    if ( g_aimbot.aim_points.empty( ) ) return;
}

void aimbot::generate_points( c_cs_player *player, lag_record *record ) {
    if ( !player->cstudio_hdr( ) || !player->cstudio_hdr( )->studio_hdr )
        return;

    for ( auto &hitbox : hitboxes ) {
        const auto bbox = player->cstudio_hdr( )->studio_hdr->hitbox( hitbox, player->hitbox_set( ) );
        const auto limb = hitbox >= csgo_hitbox::hitbox_r_thigh;

        if ( limb && math::length_2d( record->velocity ) > 1.0f )
            continue;

        float ps = static_cast< float >( g_vars.aimbot_multipoint_scale.value ) * 0.01f;

        std::vector< std::pair< vector_3d, bool > > points;

        generate_points_for_hitbox( player, record, 0, points, bbox, player->cstudio_hdr( )->studio_hdr->hitbox_set( player->hitbox_set( ) ), hitbox, ps );

        if ( points.empty( ) )
            continue;

        for ( const auto &point : points ) {
            aim_point &p = aim_points.emplace_back( );

            p.pos = point.first;
            p.center = !point.second;
            p.record = record;
            p.bullet_data = { };
            p.hb = hitbox;
        }
    }

}

bool aimbot::scan_target( c_cs_player *player, lag_record *record, aim_player &target ) {
    record->cache( );

    generate_points( player, record );

    aim_point best_point;

    bool found_point = false;

    if ( aim_points.empty( ) ) return false;

    for ( auto &point : aim_points ) {
        point.bullet_data = g_penetration.run( globals::shoot_position, point.pos, point.record->player, point.record->bones );

        if ( point.bullet_data.did_hit && point.bullet_data.out_damage >= g_vars.aimbot_min_damage.value ) {
            best_point = point;
            found_point = true;
            break;
        }
    }

    if ( !found_point )
        return false;

    g_interfaces.debug_overlay->add_box_overlay( best_point.pos, vector_3d( 0, 0, 0 ), vector_3d( 5, 5, 5 ), vector_3d( 0, 0, 0 ), 255, 255, 255, 255, 0.1 );

    best.best_point = best_point.pos;
    best.damage = best_point.bullet_data.out_damage;
    best.hitbox = best_point.hb;
    best.record = record;
    best.target = player;

    return true;
}

void aimbot::adjust_speed( c_user_cmd *cmd ) {    
    if ( !globals::local_player->alive( ) || !( globals::local_player->flags( ) & FL_ONGROUND ) || !globals::local_weapon || !globals::local_weapon->get_weapon_data( ) )
        return;

    auto quick_stop = [ & ]( ) {
        const auto target_vel = -math::normalize_angle( globals::local_player->velocity( ) ) * globals::cvars::cl_forwardspeed->get_float( );

        vector_3d angles;
        g_interfaces.engine_client->get_view_angles( angles );

        vector_3d fwd;
        math::angle_vectors( angles, &fwd );
        const auto right = glm::cross( fwd, vector_3d( 0.0f, 0.0f, 1.0f ) );

        cmd->forward_move = ( target_vel.y - ( right.y / right.x ) * target_vel.x ) / ( fwd.y - ( right.y / right.x ) * fwd.x );
        cmd->side_move = ( target_vel.x - fwd.x * cmd->forward_move ) / right.x;

        cmd->forward_move = std::clamp< float >( cmd->forward_move, -globals::cvars::cl_forwardspeed->get_float( ), globals::cvars::cl_forwardspeed->get_float( ) );
        cmd->side_move = std::clamp< float >( cmd->side_move, -globals::cvars::cl_forwardspeed->get_float( ), globals::cvars::cl_sidespeed->get_float( ) );

        cmd->buttons &= ~buttons::walk;
        cmd->buttons |= buttons::speed;
    };

    const auto speed = math::length_2d( globals::local_player->velocity( ) );

    if ( speed <= 4.0f )
        return;

    auto max_speed = globals::local_player->scoped( ) ? globals::local_weapon->get_weapon_data( )->max_speed_alt : globals::local_weapon->get_weapon_data( )->max_speed;

    const auto pure_accurate_speed = max_speed * 0.34f;
    const auto accurate_speed = max_speed * 0.315f;

    if ( speed <= pure_accurate_speed ) {
        const auto cmd_speed = sqrt( cmd->forward_move * cmd->forward_move + cmd->side_move * cmd->side_move );
        const auto local_speed = std::max( math::length_2d( globals::local_player->velocity( ) ), 0.1f );
        const auto speed_multiplier = ( local_speed / cmd_speed ) * ( accurate_speed / local_speed );

        cmd->forward_move = std::clamp< float >( cmd->forward_move * speed_multiplier, -globals::cvars::cl_forwardspeed->get_float( ), globals::cvars::cl_forwardspeed->get_float( ) );
        cmd->side_move = std::clamp< float >( cmd->side_move * speed_multiplier, -globals::cvars::cl_forwardspeed->get_float( ), globals::cvars::cl_sidespeed->get_float( ) );

        cmd->buttons &= ~buttons::walk;
        cmd->buttons |= buttons::speed;
    }
    else {
        quick_stop( );
    }
}

void aimbot::on_create_move( c_user_cmd *cmd ) {
    reset( );

    if ( !g_vars.aimbot_enable.value )
        return;

    if ( !g_interfaces.engine_client->is_in_game( ) || !g_interfaces.engine_client->is_connected( ) )
        return;

    if ( !globals::local_player || !globals::local_weapon )
        return;

    if ( !globals::local_player->alive( ) )
        return;

    const auto weapon = g_interfaces.entity_list->get_client_entity_from_handle< c_cs_weapon_base * >( globals::local_player->weapon_handle( ) );

    if ( !weapon ) return;

    const auto weapon_data = weapon->get_weapon_data( );

    if ( !weapon_data ) 
        return;

    if ( weapon_data->weapon_type == weapon_type::WEAPONTYPE_GRENADE || weapon_data->weapon_type == weapon_type::WEAPONTYPE_KNIFE || weapon->clip_1( ) < 1 )
        return;

    search_targets( );

    if ( targets.empty( ) )
        return;

    for ( auto &target : targets ) {
        hitboxes.clear( );

        if ( g_animations.lag_info[ target.entity->index( ) ].anim_records.empty( ) )
            continue;
        
        if ( g_vars.aimbot_hitboxes_head.value ) {
            hitboxes.emplace_back( hitbox_head );
            hitboxes.emplace_back( hitbox_neck );
        }

        if ( g_vars.aimbot_hitboxes_chest.value ) {
            hitboxes.emplace_back( hitbox_chest );
            hitboxes.emplace_back( hitbox_upper_chest );
        }

        if ( g_vars.aimbot_hitboxes_stomach.value )
            hitboxes.emplace_back( hitbox_body );

        if ( g_vars.aimbot_hitboxes_pelvis.value ) {
            hitboxes.emplace_back( hitbox_pelvis );
            hitboxes.emplace_back( hitbox_r_thigh );
            hitboxes.emplace_back( hitbox_l_thigh );
        }

        if ( g_vars.aimbot_hitboxes_arms.value ) {
            hitboxes.emplace_back( hitbox_l_upper_arm );
            hitboxes.emplace_back( hitbox_r_upper_arm );
            hitboxes.emplace_back( hitbox_l_forearm );
            hitboxes.emplace_back( hitbox_r_forearm );
            hitboxes.emplace_back( hitbox_l_hand );
            hitboxes.emplace_back( hitbox_r_hand );
        }

        if ( g_vars.aimbot_hitboxes_legs.value ) {
            hitboxes.emplace_back( hitbox_r_calf );
            hitboxes.emplace_back( hitbox_l_calf );
            hitboxes.emplace_back( hitbox_r_foot );
            hitboxes.emplace_back( hitbox_l_foot );
        }

        if ( hitboxes.empty( ) )
            continue;

        if ( extrapolate_target( target ) && !g_animations.lag_info[ target.entity->index( ) ].anim_records.empty( ) ) {
            auto front = &g_animations.lag_info[ target.entity->index( ) ].anim_records.front( );

            if ( !front )
                continue;

            if ( !scan_target( target.entity, front, target ) )
                continue;
        }

        else {
            auto ideal = g_resolver.find_ideal_record( &target );

            if ( !ideal )
                continue;

            if ( !scan_target( target.entity, ideal, target ) )
                continue;

            auto last = g_resolver.find_last_record( &target );

            if ( !last )
                continue;

            if ( !scan_target( target.entity, last, target ) )
                continue;
        }

    };

    if ( !best.record || !best.target || best.damage <= 0.f )
        return;

    const auto can_attack = ( ( globals::local_player->tick_base( ) * g_interfaces.global_vars->interval_per_tick ) >= weapon->next_primary_attack( ) );

    adjust_speed( cmd );

    auto backup_pose = globals::local_player->pose_parameters( )[ 12 ];

    globals::local_player->pose_parameters( )[ 12 ] = ( math::normalize_angle( globals::local_player->aim_punch( ).x * globals::cvars::weapon_recoil_scale->get_float( ), -180.0f, 180.0f ) + 90.f ) / 180.f;

    std::array< matrix_3x4, 128 > bones;

    const auto ret = g_animations.build_bones( globals::local_player, bones.data( ), g_interfaces.global_vars->curtime );

    globals::local_player->pose_parameters( )[ 12 ] = backup_pose;

    const auto state = globals::local_player->anim_state( );

    if ( ret && state )
        globals::local_player->modify_eye_position( state, &best.best_point, bones.data( ) );

    const auto targetting_record = ( best.target && best.target->alive( ) && best.record );
    const auto calc_pos = math::vector_angle( best.best_point - globals::shoot_position );

    bool should_target = g_vars.aimbot_automatic_shoot.value || cmd->buttons & buttons::attack;

    const auto backup_origin = best.target->origin( );
    const auto backup_mins = best.target->collideable( )->mins( );
    const auto backup_maxs = best.target->collideable( )->maxs( );
    const auto backup_angles = best.target->get_abs_angles( );
    const auto backup_bones = best.target->bone_cache( );

    if ( can_attack ) {
        best.record->cache( );

        if ( globals::is_targetting = targetting_record && ( should_target && hitchance( best.target, calc_pos, best.record ) ) ) {
            globals::target_index = best.target->index( );

            auto angle = math::clamp_angle( calc_pos - globals::local_player->aim_punch( ) * globals::cvars::weapon_recoil_scale->get_float( ) );

            cmd->view_angles = angle;

            if ( g_vars.aimbot_automatic_shoot.value )
                cmd->buttons |= buttons::attack;

            if ( !g_vars.aimbot_silent.value )
                g_interfaces.engine_client->set_view_angles( cmd->view_angles );

            cmd->tick_count = game::time_to_ticks( best.record->sim_time ) + game::ticks_to_time( globals::lerp_amount );

            *globals::packet = true;
        }
    }
 
    best.target->origin( ) = backup_origin;
    best.target->set_collision_bounds( backup_mins, backup_maxs );
    best.target->set_abs_angles( backup_angles );
    best.target->bone_cache( ) = backup_bones;
}