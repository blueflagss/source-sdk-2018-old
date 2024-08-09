#include "antiaim.hpp"
#include <core/config.hpp>
#include <features/animations/animation_sync.hpp>
#include <features/movement/movement.hpp>
#include <features/penetration/penetration.hpp>
#include <features/ragebot/ragebot.hpp>
#include <features/ui/input/input.hpp>
#include <random>

std::mt19937 rng( std::time( NULL ) );

void antiaim::update_manual_direction( ) {
    using_manual_dir = manual_dir != manual_direction::none;

    bool states[ 3 ] = {
            penumbra::input::key_pressed( g_vars.exploits_antiaim_manual_left.value ),
            penumbra::input::key_pressed( g_vars.exploits_antiaim_manual_right.value ),
            penumbra::input::key_pressed( g_vars.exploits_antiaim_manual_back.value ) };

    if ( g_vars.exploits_antiaim_dir_type.value != 2 ) {
        using_manual_dir = false;
        return;
    }

    auto set_antiaim_dir = [ & ]( manual_direction state ) {
        if ( manual_dir == state ) {
            manual_dir = manual_direction::none;
            using_manual_dir = false;
        } else {
            manual_dir = state;
            using_manual_dir = true;
        }
    };

    if ( states[ 0 ] ) {
        set_antiaim_dir( manual_direction::left );
        //globals::hotkeys::manual_left = false;
    }

    if ( states[ 1 ] ) {
        set_antiaim_dir( manual_direction::right );

        //globals::hotkeys::manual_right = false;
    }

    if ( states[ 2 ] ) {
        set_antiaim_dir( manual_direction::back );

        //globals::hotkeys::manual_back = false;
    }
}
bool antiaim::handle_auto_direction( c_user_cmd *cmd ) {
    if ( direction_info.left_damage <= 0.0f && direction_info.right_damage <= 0.0f ) {
        if ( direction_info.right_fraction < direction_info.left_fraction )
            dir = cmd->view_angles.y + 90.0f;
        else
            dir = cmd->view_angles.y - 90.0f;
    } else {
        if ( direction_info.left_damage > direction_info.right_damage )
            dir = cmd->view_angles.y - 90.0f;
        else
            dir = cmd->view_angles.y + 90.0f;
    }

    return true;
}

void antiaim::handle_direction( c_user_cmd *cmd ) {
    auto get_closest_target = [ & ]( ) -> c_cs_player * {
        float best_distance = std::numeric_limits< float >::max( );
        c_cs_player *best_target = nullptr;

        for ( int i = 1; i < g_interfaces.global_vars->max_clients; i++ ) {
            auto player = g_interfaces.entity_list->get_client_entity< c_cs_player * >( i );

            if ( !player || player == globals::local_player || player->team( ) == globals::local_player->team( ) || player->dormant( ) || !player->alive( ) )
                continue;

            auto distance = glm::length( player->origin( ) - globals::local_player->get_shoot_position( ) );

            if ( distance < best_distance ) {
                best_distance = distance;
                best_target = player;
            }
        }

        return best_target;
    };

    switch ( g_vars.exploits_antiaim_dir_type.value ) {
        case 0:
            dir = cmd->view_angles.y;
            break;
        case 1:
            dir = cmd->view_angles.y + 180.f;
            break;
        case 2: {
            if ( using_manual_dir ) {
                if ( manual_dir == manual_direction::back )
                    dir = cmd->view_angles.y + 180.0f;

                else if ( manual_dir == manual_direction::left )
                    dir = cmd->view_angles.y + 90.0f;

                else if ( manual_dir == manual_direction::right )
                    dir = cmd->view_angles.y - 90.0f;
            }
        } break;
        default:
            break;
    }

    if ( g_vars.exploits_antiaim_auto_direction.value ) {
        auto closest_target = get_closest_target( );

        direction_info.left_damage = 0.0f;
        direction_info.right_damage = 0.0f;
        direction_info.left_fraction = 0.0f;
        direction_info.right_fraction = 0.0f;

        if ( closest_target ) {
            vector_3d direction_1, direction_2;

            math::angle_vectors( vector_3d( 0.f, math::normalize( math::angle_vectors( globals::local_player->origin( ) - closest_target->origin( ) ).y ) - 90.f, 0.f ), &direction_1 );
            math::angle_vectors( vector_3d( 0.f, math::normalize( math::angle_vectors( globals::local_player->origin( ) - closest_target->origin( ) ).y ) + 90.f, 0.f ), &direction_2 );

            const auto left_eye_pos = closest_target->origin( ) + vector_3d( 0.f, 0.f, 64.f ) + ( direction_1 * 16.f );
            const auto right_eye_pos = closest_target->origin( ) + vector_3d( 0.f, 0.f, 64.f ) + ( direction_2 * 16.f );

            direction_info.left_damage = g_penetration.run( globals::local_player->get_shoot_position( ), left_eye_pos, closest_target, 0.0f, g_animations.animated_bones[ closest_target->index( ) ], false ).out_damage;
            direction_info.right_damage = g_penetration.run( globals::local_player->get_shoot_position( ), right_eye_pos, closest_target, 0.0f, g_animations.animated_bones[ closest_target->index( ) ], false ).out_damage;

            ray_t ray;
            c_game_trace trace;
            c_trace_filter_hitscan filter;

            filter.player = globals::local_player;

            ray.init( left_eye_pos, globals::local_player->get_shoot_position( ) );
            g_interfaces.engine_trace->trace_ray( ray, mask_all, &filter, &trace );
            direction_info.left_fraction = trace.fraction;

            ray.init( right_eye_pos, globals::local_player->get_shoot_position( ) );
            g_interfaces.engine_trace->trace_ray( ray, mask_all, &filter, &trace );
            direction_info.right_fraction = trace.fraction;

            handle_auto_direction( cmd );
        }
    }

    dir = math::normalize( dir );
}

void antiaim::handle_fake( c_user_cmd *cmd ) {
    *globals::packet = true;
    std::uniform_real_distribution gen( -90.f, 305.f );

    switch ( g_vars.exploits_antiaim_fake_yaw_type.value ) {
        case 0:
            cmd->view_angles.y = dir + 180.f;
            cmd->view_angles.y += gen( rng );
            break;
        case 1:
            cmd->view_angles.y = dir + 180.f;
            break;
        default:
            break;
    }

    cmd->view_angles.y = math::normalize( cmd->view_angles.y );
}

void antiaim::handle_real( c_user_cmd *cmd ) {
    cmd->view_angles.y = dir;

    const float range = g_vars.exploits_antiaim_range.value / 2.f;
    std::uniform_real_distribution gen( -range, range );

    if ( g_vars.exploits_antiaim_lby_break.value ) {
        const auto standing = glm::length( globals::local_player->velocity( ) ) < 1.0f;
        const auto air = ( globals::local_player->flags( ) & player_flags::on_ground ) || globals::local_player->velocity( ).z > 1.0f;

        if ( !g_interfaces.client_state->choked_commands( ) && ( g_interfaces.global_vars->curtime > g_animations.lower_body_realign_timer ) && ( standing || air ) )
            cmd->view_angles.y += g_vars.exploits_antiaim_lby_break_delta.value;

        else {
            if ( !using_manual_dir ) {
                switch ( g_vars.exploits_antiaim_yaw_type.value ) {
                    case 1:
                        cmd->view_angles.y += gen( rng );
                        break;
                    case 2:
                        cmd->view_angles.y = ( dir - g_vars.exploits_antiaim_range.value / 2.f );
                        cmd->view_angles.y +=
                                std::fmod( g_interfaces.global_vars->curtime * ( g_vars.exploits_antiaim_spin_speed.value * 100.f ),
                                           g_vars.exploits_antiaim_range.value );
                        break;
                    default:
                        break;
                }

                distortion( cmd );
                cmd->view_angles.y += g_vars.exploits_antiaim_yaw_offset.value;
            }
        }
    }

    cmd->view_angles.y = math::normalize( cmd->view_angles.y );
    real.y = cmd->view_angles.y;
}

int antiaim::get_max_choke_ticks( ) {
    if ( g_config.get_hotkey( g_vars.misc_fake_walk_key, g_vars.misc_fake_walk_key_toggle.value ) )
        return g_vars.misc_fake_walk_value.value;

    if ( glm::length( globals::local_player->velocity( ) ) < 0.18f )
        return 1.0f;

    return g_vars.exploits_fakelag_limit.value;
}

void antiaim::fake_walk( c_user_cmd *cmd ) {
    if ( !g_vars.misc_fake_walk.value )
        return;

    if ( !globals::local_player )
        return;

    auto velocity = globals::local_player->velocity( );
    int ticks{ }, max{ 16 };

    if ( !g_config.get_hotkey( g_vars.misc_fake_walk_key, g_vars.misc_fake_walk_key_toggle.value ) )
        return;

    if ( !globals::local_player->ground_entity_handle( ) )
        return;

    float friction = globals::cvars::sv_friction->get_float( ) * globals::local_player->surface_friction( );

    for ( ; ticks < get_max_choke_ticks( ); ++ticks ) {
        float speed = glm::length( velocity );

        if ( speed <= 0.1f )
            break;

        float control = std::max( speed, globals::cvars::sv_stopspeed->get_float( ) );
        float drop = control * friction * g_interfaces.global_vars->interval_per_tick;
        float newspeed = std::max( 0.f, speed - drop );

        if ( newspeed != speed ) {
            newspeed /= speed;

            velocity *= newspeed;
        }
    }

    if ( ticks > ( ( max - 1 ) - g_interfaces.client_state->choked_commands( ) ) || !g_interfaces.client_state->choked_commands( ) ) {
        g_movement.quick_stop( cmd );
    }
}

void antiaim::handle_pitch( c_user_cmd *cmd ) {
    switch ( g_vars.exploits_antiaim_pitch_type.value ) {
        case 0:
            cmd->view_angles.x = 89.f;
            break;
        case 1:
            cmd->view_angles.x = -89.f;
            break;
        case 2:
            cmd->view_angles.x = 0.f;
            break;
        default:
            break;
    }

    real.x = cmd->view_angles.x;
}

void antiaim::distortion( c_user_cmd *cmd ) {
    const auto timer = ( g_vars.exploits_antiaim_distortion_speed.value / 100.0f ) * 0.0625f;
    const auto angle = ( float ) ( ( float ) ( 1.0f - std::powf( distortion_timer, 2.0f ) ) * g_vars.exploits_antiaim_distortion_range.value ) - ( float ) ( g_vars.exploits_antiaim_distortion_range.value * 0.5 );

    distortion_timer += timer;
    cmd->view_angles.y += switch_distortion_side ? angle : -angle;

    if ( distortion_timer >= 0.69999999f ) {
        distortion_timer = 0.0f;
        switch_distortion_side = !switch_distortion_side;
    }
}

void antiaim::on_create_move( c_user_cmd *cmd, vector_3d vangle ) {
    if ( !g_vars.exploits_antiaim.value )
        return;

    if ( !g_interfaces.engine_client->is_in_game( ) || !g_interfaces.engine_client->is_connected( ) )
        return;

    if ( !globals::local_player )
        return;

    if ( !globals::local_player->alive( ) )
        return;

    if ( cmd->buttons & buttons::use || cmd->buttons & buttons::attack )
        return;

    handle_pitch( cmd );
    handle_direction( cmd );

    if ( g_vars.exploits_antiaim_fake.value ) {
        g_vars.exploits_fakelag_limit.value = std::clamp< int >( g_vars.exploits_fakelag_limit.value, 1, 16 );
        g_vars.exploits_fakelag.value = true;

        if ( *globals::packet && globals::old_packet )
            *globals::packet = false;

        if ( !*globals::packet || !globals::old_packet )
            handle_real( cmd );
        else
            handle_fake( cmd );
    } else
        handle_real( cmd );
}