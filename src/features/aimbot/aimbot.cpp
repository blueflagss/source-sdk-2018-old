#include "aimbot.hpp"

#include <core/config.hpp>
#include <features/engine_prediction/engine_prediction.hpp>
#include <features/penetration/penetration.hpp>
#include <features/resolver/resolver.hpp>
#include <features/ui/notifications/notifications.hpp>
#include <threadutils/threading.h>

bool aimbot::setup_point_for_scan( c_cs_player *player, lag_record &record, int hit_group ) {
    return true;
}

void aimbot::reset( ) {
    best = { };
    globals::target_index = -1;
    globals::is_targetting = false;
    targets.clear( );
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

                targets.emplace_back( aim_player{ player, player->index( ), player->health( ), glm::length( player->origin( ) - globals::local_player->origin( ) ), math::calculate_fov( globals::view_angles, math::clamp_angle( player->get_shoot_position( ) ) ), 0, &g_animations.lag_info[ player->index( ) ].anim_records.front( ) } );
            } break;
        }
    }

    if ( targets.empty( ) )
        return;

    std::sort( targets.begin( ), targets.end( ), [ & ]( aim_player &lhs, aim_player &rhs ) {
        if ( fabsf( lhs.fov - rhs.fov ) < 20.0f )
            return lhs.health < rhs.health;

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

bool aimbot::get_best_aim_position( aim_player &target, float &dmg, vector_3d &position, lag_record *record ) {
    bool done = false;

    if ( !record || !target.entity || target.entity->dormant( ) )
        return false;

    record->cache( );

    float scan_damage = 0.0f;
    vector_3d scan_position = { };
    int scan_hitbox = 0;

    for ( auto &hitbox : hitboxes ) {
        done = false;

        vector_3d out;

        if ( !get_hitbox_position( target.entity, record->bones.data( ), hitbox, out ) )
            continue;

        auto results = g_penetration.run( globals::shoot_position, out, target.entity, record->bones, false );

        if ( results.out_damage > g_vars.aimbot_min_damage.value ) {
            scan_damage = results.out_damage;
            scan_position = out;
            scan_hitbox = hitbox;

            for ( int i = 0; i < results.impact_count; i++ ) {
                auto pos = results.impacts[ i ];

                if ( results.impacts[ i + 1 ] != vector_3d( 0, 0, 0 ) ) {
                    g_interfaces.debug_overlay->add_line_overlay( pos, results.impacts[ i + 1 ], 255, 255, 255, true, 1 );
                    g_interfaces.debug_overlay->add_text_overlay( pos, 1, "%i | %.1f", i + 1, results.out_damage );
                } else {
                    g_interfaces.debug_overlay->add_box_overlay( pos, vector_3d( -5, -5, -5 ), vector_3d( 5, 5, 5 ), vector_3d( 0, 0, 0 ), 255, 0, 0, 255, 1 );
                    g_interfaces.debug_overlay->add_text_overlay( pos + vector_3d( 0, 20, 0 ), 1, "STOPPED %i | %.1f", i + 1, results.out_damage );
                }
            }

            break;
        }
    }

    if ( scan_damage > 0.0f ) {
        position = scan_position;
        dmg = scan_damage;
        return true;
    }

    return false;
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
    if ( !weapon_data ) return;

    if ( weapon_data->weapon_type == weapon_type::WEAPONTYPE_GRENADE || weapon_data->weapon_type == weapon_type::WEAPONTYPE_KNIFE || weapon->clip_1() < 1 ) return;

    if ( !( ( globals::local_player->tick_base( ) * g_interfaces.global_vars->interval_per_tick ) >= weapon->next_primary_attack( ) ) ) return;

    search_targets( );

    if ( targets.empty( ) )
        return;

    for ( auto &target : targets ) {
        hitboxes.clear( );

        if ( g_animations.lag_info[ target.index ].anim_records.empty( ) )
            continue;

        auto ideal = g_resolver.find_ideal_record( &target );

        if ( !ideal )
            continue;

        if ( g_vars.aimbot_hitboxes_head.value )
            hitboxes.emplace_back( hitbox_head );

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

        if ( g_vars.aimbot_hitboxes_legs.value ) {
            hitboxes.emplace_back( hitbox_r_calf );
            hitboxes.emplace_back( hitbox_l_calf );
            hitboxes.emplace_back( hitbox_r_foot );
            hitboxes.emplace_back( hitbox_l_foot );
        }

        if ( hitboxes.empty( ) )
            continue;

        float out_damage = 0.0f;

        vector_3d out_position = { };

        const auto backup_origin = target.entity->origin( );
        const auto backup_mins = target.entity->collideable( )->mins( );
        const auto backup_maxs = target.entity->collideable( )->maxs( );
        const auto backup_angles = target.entity->get_abs_angles( );
        const auto backup_bones = target.entity->bone_cache( );

        if ( get_best_aim_position( target, out_damage, out_position, ideal ) ) {
            best.target = target.entity;
            best.best_point = out_position;
            best.damage = out_damage;
            best.record = ideal;
        }

        target.entity->origin( ) = backup_origin;
        target.entity->set_collision_bounds( backup_mins, backup_maxs );
        target.entity->set_abs_angles( backup_angles );
        target.entity->bone_cache( ) = backup_bones;

        auto last = g_resolver.find_last_record( &target );

        if ( !last || last == ideal )
            continue;

        if ( get_best_aim_position( target, out_damage, out_position, last ) ) {
            best.target = target.entity;
            best.best_point = out_position;
            best.damage = out_damage;
            best.record = last;
        }

        target.entity->origin( ) = backup_origin;
        target.entity->set_collision_bounds( backup_mins, backup_maxs );
        target.entity->set_abs_angles( backup_angles );
        target.entity->bone_cache( ) = backup_bones;
    };
    
    if ( !best.record || !best.target || best.damage <= 0.f )
        return;

    /* re-calculate eye position */
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
    
    /* restore player data */
    const auto backup_origin = best.target->origin( );
    const auto backup_mins = best.target->collideable( )->mins( );
    const auto backup_maxs = best.target->collideable( )->maxs( );
    const auto backup_angles = best.target->get_abs_angles( );
    const auto backup_bones = best.target->bone_cache( );

    best.record->cache( );

    if ( globals::is_targetting = targetting_record && ( should_target && hitchance( best.target, calc_pos, best.record ) ) ) {
        globals::target_index = best.target->index( );

        auto angle = math::clamp_angle( calc_pos - globals::local_player->aim_punch( ) * globals::cvars::weapon_recoil_scale->get_float( ) );

        cmd->view_angles = angle;

        if ( g_vars.aimbot_automatic_shoot.value )
            cmd->buttons |= buttons::attack;

        if ( !g_vars.aimbot_silent.value )
            g_interfaces.engine_client->set_view_angles( cmd->view_angles );

        cmd->tick_count = game::time_to_ticks( best.record->sim_time + globals::lerp_amount );

        *globals::packet = true;
    }

    best.target->origin( ) = backup_origin;
    best.target->set_collision_bounds( backup_mins, backup_maxs );
    best.target->set_abs_angles( backup_angles );
    best.target->bone_cache( ) = backup_bones;
}