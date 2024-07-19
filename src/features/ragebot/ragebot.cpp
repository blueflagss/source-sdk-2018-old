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
    hitscan_info::best = { };
    best = { };
    targets.clear( );
}

bool ragebot::can_hit_player( c_cs_player *player, vector_3d start, vector_3d end, lag_record *record, matrix_3x4 *matrix ) {
    if ( !record || !matrix )
        return false;

    bool did_intersect_anything = false;

    for ( int i = 0; i < hitbox_max; i++ ) {
        if ( can_hit_player( player, start, end, record, i, matrix ) ) {
            did_intersect_anything = true;
            break;
        }
    }

    return did_intersect_anything;
}

bool ragebot::can_hit_player( c_cs_player *player, vector_3d start, vector_3d end, lag_record *record, int hitbox, matrix_3x4 *matrix ) {
    if ( !record )
        return false;

    if ( !matrix )
        return false;

    hitbox_data hitbox_data;

    if ( !get_hitbox_data( start, &hitbox_data, player, hitbox, matrix ) ) {
        return false;
    }

    auto dir = math::normalize_angle( end - start );

    bool intersect = false;
    if ( hitbox_data.is_obb ) {
        vector_3d delta;
        math::vector_irotate( ( dir * 8192.f ), matrix[ hitbox_data.bone ], delta );

        intersect = math::intersect_bb( hitbox_data.start_scaled, delta, hitbox_data.mins, hitbox_data.maxs );
    } else {
        intersect = math::intersect( start, end, hitbox_data.mins, hitbox_data.maxs, hitbox_data.radius );
    }

    if ( !intersect )
        return false;

    // https://www.unknowncheats.me/forum/counterstrike-global-offensive/345397-performance-raytracer.html
    if ( !hitbox_data.is_obb ) {
        RayTracer::Ray ray( start, end );// what about hitbox_data.m_start_scaled
        RayTracer::Trace trace;
        RayTracer::Hitbox trace_hitbox( hitbox_data.mins, hitbox_data.maxs, hitbox_data.radius );
        RayTracer::TraceHitbox( ray, trace_hitbox, trace );

        if ( !trace.m_hit ) {
            return false;
        }
    }

    return true;
}

bool ragebot::get_hitbox_data( vector_3d start, hitbox_data *rtn, c_cs_player *player, int hitbox, matrix_3x4 *matrix ) {
    if ( hitbox < csgo_hitbox::hitbox_head || hitbox > csgo_hitbox::hitbox_max )
        return false;

    if ( !player || !matrix )
        return false;

    auto model = player->get_model( );

    if ( !model )
        return false;

    studiohdr_t *hdr = g_interfaces.model_info->get_studio_model( model );
    if ( !hdr )
        return false;

    mstudiohitboxset_t *set = hdr->hitbox_set( player->hitbox_set( ) );
    if ( !set )
        return false;

    mstudiobbox_t *bbox = set->hitbox( hitbox );
    if ( !bbox )
        return false;

    const auto is_capsule = bbox->radius != -1.f;

    vector_3d m_min, m_max;
    if ( is_capsule ) {
        math::vector_transform( bbox->min, matrix[ bbox->bone ], m_min );
        math::vector_transform( bbox->max, matrix[ bbox->bone ], m_max );
    } else {
        m_min = math::vector_rotate( bbox->min, bbox->angle );
        m_max = math::vector_rotate( bbox->max, bbox->angle );
        math::vector_transform( m_min, matrix[ bbox->bone ], m_min );
        math::vector_transform( m_max, matrix[ bbox->bone ], m_max );
    }

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

bool ragebot::should_hit( c_cs_player *player, const vector_3d &angle, lag_record *record ) {
    /* https://github.com/OneshotGH/supremacy/blob/9d56ba52f4ab380f48ca3f066c9c5009ea2bb74f/aimbot.cpp#L658 */
    vector_3d fwd, right, up;
    size_t total_hits{ }, needed_hits{ ( size_t ) std::ceil( ( g_vars.aimbot_hit_chance.value * 255 ) / 100 ) };

    math::angle_vectors( angle, &fwd, &right, &up );

    auto inaccuracy = globals::local_weapon->get_inaccuracy( );
    auto spread = globals::local_weapon->get_spread( );

    for ( int i{ }; i <= 255; ++i ) {
        auto spread_direction = globals::local_weapon->calculate_spread( i, inaccuracy, spread );
        auto dir = math::normalize_angle( fwd + ( right * spread_direction.x ) + ( up * spread_direction.y ) );
        auto end = globals::local_player->get_shoot_position( ) + ( dir * globals::local_weapon_data->range );

        ray_t ray;
        ray.init( globals::local_player->get_shoot_position( ), end );

        c_game_trace tr;
        g_interfaces.engine_trace->clip_ray_to_entity( ray, mask_shot, player, &tr );

        if ( tr.entity == player && ( ( tr.hit_group >= hitgroups::hitgroup_head && tr.hit_group <= hitgroups::hitgroup_rightleg ) || tr.hit_group == hitgroups::hitgroup_gear ) )
            ++total_hits;

        if ( total_hits >= needed_hits )
            return true;

        if ( ( 255 - i + total_hits ) < needed_hits )
            return false;
    }

    return false;
}

bool ragebot::get_hitbox_position( c_cs_player *player, matrix_3x4 *bones, int i, vector_3d &position ) {
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

void ragebot::search_targets( ) {
    for ( int i = 1; i <= ( g_interfaces.entity_list->get_highest_entity_index( ) + 1 ); ++i ) {
        const auto entity = g_interfaces.entity_list->get_client_entity< c_cs_player * >( i );

        if ( !entity || !entity->is_player( ) || !entity->alive( ) || entity->dormant( ) || entity->team( ) == globals::local_player->team( ) || entity == globals::local_player ) {
            continue;
        }

        if ( g_animations.player_log[ entity->index( ) ].anim_records.empty( ) )
            continue;

        //players[ entity->index( ) ] = target;
        targets.push_back( aim_player{ entity, math::calculate_fov( globals::view_angles, math::clamp_angle( entity->origin( ) ) ), glm::length( entity->origin( ) - globals::local_player->origin( ) ) } );
    }

    if ( targets.empty( ) )
        return;

    std::sort( targets.begin( ), targets.end( ), [ & ]( aim_player &lhs, aim_player &rhs ) {
        switch ( g_vars.aimbot_sort_by.value ) {
            case 0:
                return lhs.distance < rhs.distance;
                break;
            case 1:
                return lhs.fov < rhs.fov;
                break;
        }
    } );
}

std::vector< int > ragebot::get_hitboxes( ) {
    std::vector< int > hitboxes{ };

    hitboxes.clear( );

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

void run_hitscan( thread_args *args ) {
    if ( !args || !args->valid || reinterpret_cast< uintptr_t >( args->target ) == 0xCCCCCCCC || args->hb > hitbox_max )
        return;

    const auto record = args->record;
    const auto player = args->target;

    if ( !g_ragebot.should_continue_thread ) {
        args->done = false;
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

    mstudiohitboxset_t *set = hdr->hitbox_set( player->hitbox_set( ) );

    if ( !set )
        return;

    std::array< std::vector< aim_point >, 19 > aim_points = { };

    float best_damage = 0.0f;

    record->cache( );

    for ( auto &hb : g_ragebot.get_hitboxes( ) ) {
        args->points.clear( );

        mstudiobbox_t *bbox = set->hitbox( hb );

        if ( !bbox )
            continue;

        const auto limb = hb >= csgo_hitbox::hitbox_r_thigh;

        if ( limb && math::length_2d( record->velocity ) > 1.0f )
            continue;

        const auto center = ( bbox->max + bbox->min ) * 0.5f;
        const auto angle = math::vector_angle( center - globals::local_player->get_shoot_position( ) );

        vector_3d forward;
        math::angle_vectors( angle, &forward );

        vector_3d center_transformed;
        math::vector_transform( center, record->bones[ bbox->bone ], center_transformed );
        args->points.push_back( std::make_pair( center_transformed, false ) );

        if ( ps <= 0.0f )
            return;

        if ( bbox->radius <= 0.0f ) {
            if ( hb == csgo_hitbox::hitbox_r_foot || hb == csgo_hitbox::hitbox_l_foot ) {
                float d1 = ( bbox->min.z - center.z ) * 0.425f;

                if ( hb == csgo_hitbox::hitbox_l_foot )
                    d1 *= -1.f;

                vector_3d toe = vector_3d{ ( ( bbox->max.x - center.x ) * ps ) + center.x, center.y, center.z };
                vector_3d heel = vector_3d{ ( ( bbox->min.x - center.x ) * ps ) + center.x, center.y, center.z };

                args->points.push_back( std::make_pair( math::vector_transform( toe, record->bones[ bbox->bone ] ), true ) );
                args->points.push_back( std::make_pair( math::vector_transform( heel, record->bones[ bbox->bone ] ), true ) );
            }
        } else {
            float r = bbox->radius * ps;

            if ( hb == csgo_hitbox::hitbox_head ) {
                vector_3d left{ bbox->max.x, bbox->max.y, bbox->max.z - ( bbox->radius * 0.5f ) };
                args->points.push_back( std::make_pair( math::vector_transform( left, record->bones[ bbox->bone ] ), true ) );

                vector_3d right{ bbox->max.x, bbox->max.y, bbox->max.z + ( bbox->radius * 0.5f ) };
                args->points.push_back( std::make_pair( math::vector_transform( right, record->bones[ bbox->bone ] ), true ) );

                constexpr float rotation = 0.70710678f;

                ps = std::clamp< float >( ps, 0.1f, 0.95f );
                r = bbox->radius * ps;

                vector_3d topback{ bbox->max.x + ( rotation * r ), bbox->max.y + ( -rotation * r ), bbox->max.z };
                args->points.push_back( std::make_pair( math::vector_transform( topback, record->bones[ bbox->bone ] ), true ) );
                args->points.push_back( std::make_pair( math::vector_transform( vector_3d( center.x, bbox->max.y - r, center.z ), record->bones[ bbox->bone ] ), true ) );
            }
            if ( hb == csgo_hitbox::hitbox_body || hb == csgo_hitbox::hitbox_pelvis ) {
                vector_3d back{ center.x, bbox->max.y - r, center.z };
                vector_3d right{ bbox->max.x, bbox->max.y, bbox->max.z + ( bbox->radius * 0.5f ) };
                vector_3d left{ bbox->max.x, bbox->max.y, bbox->max.z - ( bbox->radius * 0.5f ) };

                args->points.push_back( std::make_pair( math::vector_transform( back, record->bones[ bbox->bone ] ), true ) );
                args->points.push_back( std::make_pair( math::vector_transform( right, record->bones[ bbox->bone ] ), true ) );
                args->points.push_back( std::make_pair( math::vector_transform( left, record->bones[ bbox->bone ] ), true ) );
            }
            if ( hb == csgo_hitbox::hitbox_thorax || hb == csgo_hitbox::hitbox_chest || hb == csgo_hitbox::hitbox_upper_chest ) {
                vector_3d back{ center.x, bbox->max.y - r, center.z };
                args->points.push_back( std::make_pair( math::vector_transform( back, record->bones[ bbox->bone ] ), true ) );
            }

            if ( hb == csgo_hitbox::hitbox_r_thigh || hb == csgo_hitbox::hitbox_l_thigh ) {
                vector_3d half_bottom{ bbox->max.x - ( bbox->radius * 0.5f ), bbox->max.y, bbox->max.z };
                args->points.push_back( std::make_pair( math::vector_transform( half_bottom, record->bones[ bbox->bone ] ), true ) );
            }
        }

        if ( args->points.empty( ) )
            continue;

        for ( auto &p : args->points ) {
            auto bullet_data = g_penetration.run( globals::local_player->get_shoot_position( ), p.first, player, record->bones );

            if ( bullet_data.did_hit ) {
                //if ( hb == hitbox_head && bullet_data.out_hitgroup != hitgroup_head )
                //    continue;

                if ( bullet_data.out_damage >= best_damage ) {
                    best_damage = bullet_data.out_damage;

                    hitscan_info::best.hitbox = hb;
                    hitscan_info::best.best_point = p.first;
                    hitscan_info::best.record = record;
                    hitscan_info::best.target = player;
                    hitscan_info::best.damage = bullet_data.out_damage;
                    args->done = true;
                }

                if ( args->done && bullet_data.out_damage >= best_damage ) {
                    hitscan_info::best.hitbox = hb;
                    hitscan_info::best.best_point = p.first;
                    hitscan_info::best.damage = bullet_data.out_damage;
                    break;
                }

                if ( p == args->points.front( ) && bullet_data.out_damage >= player->health( ) )
                    break;    
            }  

            if ( args->done ) {
                g_ragebot.should_continue_thread = false;
                break;
            }
        }
    }

    if ( args->done )
        g_ragebot.should_continue_thread = false;
}

void ragebot::generate_points( c_cs_player *player, lag_record *record ) {
}

bool ragebot::scan_target( c_cs_player *player, lag_record *record, aim_player &target ) {
    thread_args args;

    args.record = record;
    args.target = player;
    args.done = false;
    args.valid = true;
    args.points = { };
    args.hb = hitbox_l_foot;
    should_continue_thread = true;
    //run_hitscan( &args );
    Threading::QueueJobRef( run_hitscan, ( void * )&args );
    
    best = hitscan_info::best;

    Threading::FinishQueue( true );

    return true;
}

void ragebot::adjust_speed( c_user_cmd *cmd ) { /* kinda shit. */
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
        cmd->side_move = std::clamp< float >( cmd->side_move, -globals::cvars::cl_forwardspeed->get_float( ), globals::cvars::cl_sidespeed->get_float( ) );

        cmd->buttons &= ~buttons::walk;
        cmd->buttons |= buttons::speed;
    };

    const auto speed = math::length_2d( globals::local_player->velocity( ) );

    if ( speed <= 4.0f )
        return;

    auto max_speed = globals::local_player->scoped( ) ? globals::local_weapon_data->max_speed_alt : globals::local_weapon_data->max_speed;

    const auto pure_accurate_speed = max_speed * 0.34f;
    const auto accurate_speed = max_speed * 0.315f;

    //    actually slowwalk
    if ( speed <= pure_accurate_speed ) {
        const auto cmd_speed = sqrt( cmd->forward_move * cmd->forward_move + cmd->side_move * cmd->side_move );
        const auto local_speed = std::max( math::length_2d( globals::local_player->velocity( ) ), 0.1f );
        const auto speed_multiplier = ( local_speed / cmd_speed ) * ( accurate_speed / local_speed );

        cmd->forward_move = std::clamp< float >( cmd->forward_move * speed_multiplier, -globals::cvars::cl_forwardspeed->get_float( ), globals::cvars::cl_forwardspeed->get_float( ) );
        cmd->side_move = std::clamp< float >( cmd->side_move * speed_multiplier, -globals::cvars::cl_forwardspeed->get_float( ), globals::cvars::cl_sidespeed->get_float( ) );

        cmd->buttons &= ~buttons::walk;
        cmd->buttons |= buttons::speed;
    }
    //    we are fast
    else {
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

    search_targets( );

    if ( targets.empty( ) )
        return;

    for ( auto &target : targets ) {
        if ( !target.entity )
            continue;

        auto &info = g_animations.player_log[ target.entity->index( ) ];

        if ( info.anim_records.empty( ) )
            continue;

        const auto front = &g_animations.player_log[ target.entity->index( ) ].anim_records.front( );
        const auto bot = info.player_info.fake_player;

        if ( !bot && front && g_animations.should_predict_lag( target, front, nullptr ) ) {
            if ( !scan_target( target.entity, front, target ) )
                continue;
        }

        else {
            //if ( target.delay_shot )
            //    continue;

            auto ideal = g_resolver.find_ideal_record( target.entity );

            if ( !ideal )
                continue;

            if ( !scan_target( target.entity, ideal, target ) )
                continue;

            auto last_record = g_resolver.find_last_record( target.entity );

            if ( !last_record )
                continue;

            if ( !scan_target( target.entity, last_record, target ) )
                continue;
        }
    }

    if ( !best.record || !best.target || best.damage <= 0 )
        return;

    adjust_speed( cmd );

    bool should_target = ( g_vars.aimbot_automatic_shoot.value );

    /* restore player data */
    const auto backup_origin = best.target->origin( );
    const auto backup_mins = best.target->collideable( )->mins( );
    const auto backup_maxs = best.target->collideable( )->maxs( );
    const auto backup_angles = best.target->get_abs_angles( );
    const auto backup_bones = best.target->bone_cache( );

    best.record->cache( );

    const auto targetting_record = ( best.target && best.target->alive( ) && best.record );
    const auto calc_pos = math::vector_angle( best.best_point - globals::local_player->get_shoot_position( ) );

    if ( should_target && should_hit( best.target, calc_pos, best.record ) ) {
        globals::target_index = best.target->index( );

        cmd->view_angles = math::clamp_angle( calc_pos - globals::local_player->aim_punch( ) * globals::cvars::weapon_recoil_scale->get_float( ) );
        cmd->tick_count = game::time_to_ticks( best.record->sim_time ) + game::time_to_ticks( globals::lerp_amount );

        if ( g_vars.aimbot_automatic_shoot.value )
            cmd->buttons |= buttons::attack;

        if ( !g_vars.aimbot_silent.value )
            g_interfaces.engine_client->set_view_angles( cmd->view_angles );

        g_shot_manager.on_shot_fire( best.target ? best.target : nullptr, best.target ? best.damage : -1.f, globals::local_weapon->get_weapon_data( )->bullets, best.target ? best.record : nullptr );

        *globals::packet = true;
    }

    best.target->origin( ) = backup_origin;
    best.target->set_collision_bounds( backup_mins, backup_maxs );
    best.target->set_abs_angles( backup_angles );
    best.target->bone_cache( ) = backup_bones;
}