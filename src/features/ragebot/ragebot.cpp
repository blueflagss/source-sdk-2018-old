#include "ragebot.hpp"
#include <core/config.hpp>
#include <features/prediction_manager/prediction_manager.hpp>
#include <features/resolver/resolver.hpp>
#include <features/shot_manager/shot_manager.hpp>
#include <threadutils/threading.h>
#include <utils/threading/dispatch.hpp>

void ragebot::reset( ) {
    globals::target_index = -1;
    globals::is_targetting = false;
    best = { };
    targets.clear( );
    hitscan_info::best = { };
}

void ragebot::search_targets( ) {
    for ( int i = 1; i <= g_interfaces.entity_list->get_highest_entity_index( ); ++i ) {
        const auto entity = g_interfaces.entity_list->get_client_entity< c_cs_player * >( i );

        if ( !entity || entity == globals::local_player || entity->team( ) == globals::local_player->team( ) || !entity->is_player( ) || !entity->alive( ) || entity->dormant( ) )
            continue;

        if ( g_animations.player_log[ entity->index( ) ].anim_records.empty( ) )
            continue;

        targets.push_back( aim_player{ entity, math::calculate_fov( globals::view_angles, math::clamp_angle( entity->origin( ) ) ), glm::length( entity->origin( ) - globals::local_player->origin( ) ) } );
    }

    if ( targets.empty( ) )
        return;

    std::sort( targets.begin( ), targets.end( ), [ & ]( aim_player &lhs, aim_player &rhs ) {
        switch ( g_vars.aimbot_sort_by.value ) {
            case 0:
                return lhs.distance < rhs.distance;
            case 1:
                return lhs.fov < rhs.fov;
        }
    } );
}

bool ragebot::can_hit( c_cs_player *player, vector_3d start, vector_3d end, lag_record *record, matrix_3x4 *matrix ) {
    if ( !record || !matrix )
        return false;

    bool found = false;

    for ( int i = 0; i < hitbox_max; i++ ) {
        const auto hdr = g_interfaces.model_info->get_studio_model( player->get_model( ) );

        if ( !hdr )
            return false;

        const auto set = hdr->hitbox_set( 0 );

        if ( !set )
            return false;

        const auto bbox = set->hitbox( i );

        if ( !bbox )
            return false;

        vector_3d min, max;

        math::vector_transform( bbox->min, matrix[ bbox->bone ], min );
        math::vector_transform( bbox->max, matrix[ bbox->bone ], max );

        if ( g_penetration.trace_ray( min, max, matrix[ bbox->bone ], bbox->radius, start, end ) ) {
            found = true;
            break;
        }
    }

    return found;
}

bool ragebot::get_hitbox_data( vector_3d start, hitbox_data *rtn, c_cs_player *player, int hitbox, matrix_3x4 *matrix ) {
    if ( hitbox < csgo_hitbox::hitbox_head || hitbox > csgo_hitbox::hitbox_max )
        return false;

    if ( !player )
        return false;

    auto model = player->get_model( );

    if ( !model )
        return false;

    const auto hdr = g_interfaces.model_info->get_studio_model( model );

    if ( !hdr )
        return false;

    const auto set = hdr->hitbox_set( 0 );

    if ( !set )
        return false;

    const auto bbox = set->hitbox( hitbox );

    if ( !bbox )
        return false;

    const auto is_capsule = bbox->radius != -1.f;

    vector_3d m_min, m_max;

    math::vector_transform( bbox->min, matrix[ bbox->bone ], m_min );
    math::vector_transform( bbox->max, matrix[ bbox->bone ], m_max );

    rtn->hitbox_id = hitbox;
    rtn->is_obb = !is_capsule;
    rtn->radius = bbox->radius;
    rtn->mins = m_min;
    rtn->maxs = m_max;
    rtn->hitgroup = bbox->group;
    rtn->hitbox = bbox;

    math::vector_transform( start, matrix[ bbox->bone ], rtn->start_scaled );

    rtn->bone = bbox->bone;

    return true;
}

bool ragebot::calculate_hitchance( lag_record *record, const int &hitbox, const vector_3d &angle, const float &htc ) {
    if ( !record || !globals::local_weapon_data )
        return false;

    const auto studio_model = g_interfaces.model_info->get_studio_model( record->player->get_model( ) );

    if ( !studio_model )
        return false;

    const auto hitbox_set = studio_model->hitbox_set( 0 );

    if ( !hitbox_set )
        return false;

    const auto bbox = hitbox_set->hitbox( hitbox );

    if ( !bbox )
        return false;

    int needed_hits = 0;

    vector_3d min, max;
    math::vector_transform( bbox->min, record->bones[ bbox->bone ], min );
    math::vector_transform( bbox->max, record->bones[ bbox->bone ], max );

    vector_3d forward{ }, right{ }, up{ };

    math::angle_vectors( angle, &forward, &right, &up );

    const auto inaccuracy = globals::local_weapon->get_inaccuracy( );
    const auto spread = globals::local_weapon->get_spread( );

    for ( int i = 0; i < 256; i++ ) {
        const auto weapon_spread = globals::local_weapon->calculate_spread( i, inaccuracy, spread );

        auto dir = forward + ( right * weapon_spread.x ) + ( up * weapon_spread.y );
        auto start = globals::local_player->get_shoot_position( );
        auto end = start + ( dir * globals::local_weapon_data->range );

        if ( g_penetration.trace_ray( min, max, record->bones[ bbox->bone ], bbox->radius, start, end ) ) {
            ++needed_hits;
        }
    }

    auto total_hits = static_cast< float >( needed_hits ) / 256.0f * 100.0f;

    if ( total_hits < htc )
        return false;

    return true;
}

std::vector< int > ragebot::get_hitboxes( ) {
    std::vector< int > hitboxes;

    if ( g_vars.aimbot_hitboxes_head.value ) {
        hitboxes.push_back( hitbox_head );
    }

    if ( g_vars.aimbot_hitboxes_chest.value ) {
        hitboxes.push_back( hitbox_thorax );
        hitboxes.push_back( hitbox_chest );
        hitboxes.push_back( hitbox_upper_chest );
    }

    if ( g_vars.aimbot_hitboxes_stomach.value ) {
        hitboxes.push_back( hitbox_body );
        hitboxes.push_back( hitbox_pelvis );
    }

    if ( g_vars.aimbot_hitboxes_arms.value ) {
        hitboxes.push_back( hitbox_l_upper_arm );
        hitboxes.push_back( hitbox_r_upper_arm );
    }

    if ( g_vars.aimbot_hitboxes_legs.value ) {
        hitboxes.push_back( hitbox_r_thigh );
        hitboxes.push_back( hitbox_l_thigh );
        hitboxes.push_back( hitbox_r_calf );
        hitboxes.push_back( hitbox_l_calf );
        hitboxes.push_back( hitbox_r_foot );
        hitboxes.push_back( hitbox_l_foot );
    }

    return hitboxes;
}

void hitscan_thread( hitscan_data *data ) {
    if ( !data || !data->valid || reinterpret_cast< uintptr_t >( data->target ) == 0xCCCCCCCC || data->hb > hitbox_max )
        return;

    const auto record = data->record;
    const auto player = data->target;

    if ( !g_ragebot.should_continue_thread ) {
        data->done = false;
        return;
    }

    if ( !player || !player->alive( ) || player->immunity( ) || !record )
        return;

    auto ps = static_cast< float >( g_vars.aimbot_multipoint_scale.value ) * 0.01f;
    auto model = player->get_model( );

    if ( !model )
        return;

    studiohdr_t *hdr = g_interfaces.model_info->get_studio_model( model );

    if ( !hdr )
        return;

    mstudiohitboxset_t *set = hdr->hitbox_set( 0 );

    if ( !set )
        return;

    std::array< std::vector< aim_point >, 19 > aim_points = { };

    float best_damage = 0.0f;

    record->cache( );

    for ( auto &hitbox : g_ragebot.get_hitboxes( ) ) {
        data->points.clear( );

        mstudiobbox_t *bbox = set->hitbox( hitbox );

        if ( !bbox )
            continue;

        const auto limb = hitbox >= csgo_hitbox::hitbox_r_thigh;

        if ( limb && math::length_2d( record->anim_velocity ) > 1.0f )
            continue;

        const auto center = ( bbox->max + bbox->min ) * 0.5f;
        const auto angle = math::vector_angle( center - globals::local_player->get_shoot_position( ) );

        vector_3d forward;
        math::angle_vectors( angle, &forward );

        vector_3d center_transformed;
        math::vector_transform( center, record->bones[ bbox->bone ], center_transformed );

        data->points.push_back( std::make_pair( center_transformed, false ) );
        data->hb = hitbox;

        if ( ps <= 0.0f )
            continue;

        if ( bbox->radius <= 0.0f ) {
            if ( hitbox == csgo_hitbox::hitbox_r_foot || hitbox == csgo_hitbox::hitbox_l_foot ) {
                float d1 = ( bbox->min.z - center.z ) * 0.425f;

                if ( hitbox == csgo_hitbox::hitbox_l_foot )
                    d1 *= -1.f;

                vector_3d toe = vector_3d{ ( ( bbox->max.x - center.x ) * ps ) + center.x, center.y, center.z };
                vector_3d heel = vector_3d{ ( ( bbox->min.x - center.x ) * ps ) + center.x, center.y, center.z };

                data->points.push_back( std::make_pair( math::vector_transform( toe, record->bones[ bbox->bone ] ), true ) );
                data->points.push_back( std::make_pair( math::vector_transform( heel, record->bones[ bbox->bone ] ), true ) );
            }
        } else {
            float r = bbox->radius * ps;

            if ( hitbox == csgo_hitbox::hitbox_head ) {
                vector_3d left{ bbox->max.x, bbox->max.y, bbox->max.z - ( bbox->radius * 0.5f ) };
                data->points.push_back( std::make_pair( math::vector_transform( left, record->bones[ bbox->bone ] ), true ) );

                vector_3d right{ bbox->max.x, bbox->max.y, bbox->max.z + ( bbox->radius * 0.5f ) };
                data->points.push_back( std::make_pair( math::vector_transform( right, record->bones[ bbox->bone ] ), true ) );

                constexpr float rotation = 0.70710678f;

                ps = std::clamp< float >( ps, 0.1f, 0.95f );
                r = bbox->radius * ps;

                vector_3d topback{ bbox->max.x + ( rotation * r ), bbox->max.y + ( -rotation * r ), bbox->max.z };
                data->points.push_back( std::make_pair( math::vector_transform( topback, record->bones[ bbox->bone ] ), true ) );
                data->points.push_back( std::make_pair( math::vector_transform( vector_3d( center.x, bbox->max.y - r, center.z ), record->bones[ bbox->bone ] ), true ) );
            }
            if ( hitbox == csgo_hitbox::hitbox_body || hitbox == csgo_hitbox::hitbox_pelvis ) {
                vector_3d back{ center.x, bbox->max.y - r, center.z };
                vector_3d right{ bbox->max.x, bbox->max.y, bbox->max.z + ( bbox->radius * 0.5f ) };
                vector_3d left{ bbox->max.x, bbox->max.y, bbox->max.z - ( bbox->radius * 0.5f ) };

                data->points.push_back( std::make_pair( math::vector_transform( back, record->bones[ bbox->bone ] ), true ) );
                data->points.push_back( std::make_pair( math::vector_transform( right, record->bones[ bbox->bone ] ), true ) );
                data->points.push_back( std::make_pair( math::vector_transform( left, record->bones[ bbox->bone ] ), true ) );
            }
            if ( hitbox == csgo_hitbox::hitbox_thorax || hitbox == csgo_hitbox::hitbox_chest || hitbox == csgo_hitbox::hitbox_upper_chest ) {
                vector_3d back{ center.x, bbox->max.y - r, center.z };
                data->points.push_back( std::make_pair( math::vector_transform( back, record->bones[ bbox->bone ] ), true ) );
            }

            if ( hitbox == csgo_hitbox::hitbox_r_thigh || hitbox == csgo_hitbox::hitbox_l_thigh ) {
                vector_3d half_bottom{ bbox->max.x - ( bbox->radius * 0.5f ), bbox->max.y, bbox->max.z };
                data->points.push_back( std::make_pair( math::vector_transform( half_bottom, record->bones[ bbox->bone ] ), true ) );
            }
        }

        if ( data->points.empty( ) )
            continue;

        for ( const auto &p : data->points ) {
            auto bullet_data = g_penetration.run( globals::local_player->get_shoot_position( ), p.first, player, g_vars.aimbot_min_damage.value, record->bones );

            if ( bullet_data.out_damage > hitscan_info::best.damage ) {
                hitscan_info::best.damage = bullet_data.out_damage;
                hitscan_info::best.hitbox = hitbox;
                hitscan_info::best.best_point = p.first;
                hitscan_info::best.record = record;
                hitscan_info::best.target = player;
                data->done = true;
            }

            if ( data->done && bullet_data.out_damage >= hitscan_info::best.damage ) {
                hitscan_info::best.damage = bullet_data.out_damage;
                hitscan_info::best.record = record;
                hitscan_info::best.target = player;
                break;
            }
        }

        if ( data->done ) {
            g_ragebot.should_continue_thread = false;
            break;
        }
    }

    if ( data->done )
        g_ragebot.should_continue_thread = false;
}

bool ragebot::scan_target( c_cs_player *player, lag_record *record, aim_player &target ) {
    hitscan_data args;

    args.record = record;
    args.target = player;
    args.done = false;
    args.valid = true;
    args.points = { };
    args.hb = hitbox_l_foot;

    should_continue_thread = true;
    best.target = player;
    best.record = record;

    Threading::QueueJobRef( hitscan_thread, ( void * ) &args );
    //run_hitscan( &args );
    Threading::FinishQueue( true );

    return true;
}

void ragebot::adjust_speed( c_user_cmd *cmd ) {
    if ( !globals::local_player->alive( ) || !( globals::local_player->flags( ) & FL_ONGROUND ) || !globals::local_weapon || !globals::local_weapon_data )
        return;

    auto quick_stop = [ & ]( ) {
        const auto target_vel = -math::normalize_angle( globals::local_player->velocity( ) ) * globals::cvars::cl_forwardspeed->get_float( );

        vector_3d fwd;
        math::angle_vectors( globals::view_angles, &fwd );
        const auto right = glm::cross( fwd, vector_3d( 0.0f, 0.0f, 1.0f ) );

        cmd->forward_move = ( target_vel.y - ( right.y / right.x ) * target_vel.x ) / ( fwd.y - ( right.y / right.x ) * fwd.x );
        cmd->side_move = ( target_vel.x - fwd.x * cmd->forward_move ) / right.x;

        cmd->forward_move = std::clamp< float >( cmd->forward_move, -globals::cvars::cl_forwardspeed->get_float( ), globals::cvars::cl_forwardspeed->get_float( ) );
        cmd->side_move = std::clamp< float >( cmd->side_move, -globals::cvars::cl_sidespeed->get_float( ), globals::cvars::cl_sidespeed->get_float( ) );

        cmd->buttons &= ~buttons::walk;
        cmd->buttons |= buttons::speed;
    };

    const auto speed = math::length_2d( globals::local_player->velocity( ) );

    if ( speed <= 4.0f )
        return;

    auto max_speed = globals::local_player->scoped( ) ? globals::local_weapon_data->max_speed_alt : globals::local_weapon_data->max_speed;

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
    } else {
        quick_stop( );
    }
}

void ragebot::on_create_move( c_user_cmd *cmd ) {
    reset( );

    if ( !g_vars.aimbot_enable.value )
        return;

    if ( !g_interfaces.engine_client->is_in_game( ) || !g_interfaces.engine_client->is_connected( ) )
        return;

    if ( !globals::local_player )
        return;

    if ( !globals::local_player->alive( ) )
        return;

    if ( !globals::local_weapon || !globals::local_weapon_data )
        return;

    if ( globals::local_weapon_data->weapon_type == weapon_type::WEAPONTYPE_GRENADE || globals::local_weapon_data->weapon_type == weapon_type::WEAPONTYPE_KNIFE )
        return;

    if ( !globals::local_player->can_attack( ) )
        return;

    if ( cmd->buttons & buttons::attack )
        return;

    search_targets( );

    if ( targets.empty( ) )
        return;

    for ( auto &target : targets ) {
        if ( !target.entity )
            continue;

        auto &info = g_animations.player_log[ target.entity->index( ) ];

        if ( info.lag_records.empty( ) )
            continue;

        const auto front = g_animations.player_log[ target.entity->index( ) ].lag_records.front( );
        const auto bot = info.player_info.fake_player;

        if ( front ) {
            if ( !bot && g_animations.should_predict_lag( target, front, nullptr ) ) {
                if ( !scan_target( target.entity, front, target ) )
                    continue;
            }

            else {
                if ( target.delay_shot && g_vars.aimbot_delay_shot.value )
                    continue;

                auto ideal = g_resolver.find_ideal_record( target.entity );

                if ( !ideal )
                    continue;

                if ( !scan_target( target.entity, ideal, target ) )
                    continue;
            }
        }
    }

    if ( !hitscan_info::best.record && !hitscan_info::best.target )
        return;

    if (hitscan_info::best.damage >= g_vars.aimbot_min_damage.value ) {

        adjust_speed( cmd );

        bool should_target = g_vars.aimbot_automatic_shoot.value;

        const auto backup_origin = hitscan_info::best.target->origin( );
        const auto backup_mins = hitscan_info::best.target->collideable( )->mins( );
        const auto backup_maxs = hitscan_info::best.target->collideable( )->maxs( );
        const auto backup_angles = hitscan_info::best.target->get_abs_angles( );
        const auto backup_bones = hitscan_info::best.target->bone_cache( );
        //const auto backup_poses = hitscan_info::best.target->pose_parameters( );

        hitscan_info::best.record->cache( );

        const auto targetting_record = ( hitscan_info::best.target && hitscan_info::best.target->alive( ) && hitscan_info::best.record );
        const auto calc_pos = math::vector_angle( hitscan_info::best.best_point - globals::local_player->get_shoot_position( ) );

        if ( should_target && calculate_hitchance( hitscan_info::best.record, hitscan_info::best.hitbox, calc_pos, g_vars.aimbot_hit_chance.value ) ) {
            globals::target_index = hitscan_info::best.target->index( );

            cmd->tick_count = game::time_to_ticks( hitscan_info::best.record->sim_time + globals::lerp_amount );
            cmd->view_angles = math::clamp_angle( calc_pos - globals::local_player->aim_punch( ) * globals::cvars::weapon_recoil_scale->get_float( ) );

            if ( g_vars.aimbot_automatic_shoot.value )
                cmd->buttons |= buttons::attack;

            if ( !g_vars.aimbot_silent.value )
                g_interfaces.engine_client->set_view_angles( cmd->view_angles );

            g_shot_manager.on_shot_fire( hitscan_info::best.target ? hitscan_info::best.target : nullptr, hitscan_info::best.target ? hitscan_info::best.damage : -1.f, globals::local_weapon->get_weapon_data( )->bullets, hitscan_info::best.target ? hitscan_info::best.record : nullptr );

            *globals::packet = true;
        }

        hitscan_info::best.target->origin( ) = backup_origin;
        hitscan_info::best.target->set_collision_bounds( backup_mins, backup_maxs );
        hitscan_info::best.target->set_abs_angles( backup_angles );
        hitscan_info::best.target->bone_cache( ) = backup_bones;
        //hitscan_info::best.target->pose_parameters( ) = backup_poses;
    }
}