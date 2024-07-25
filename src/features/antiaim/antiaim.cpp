#include "antiaim.hpp"
#include <core/config.hpp>
#include <features/animations/animation_sync.hpp>
#include <features/movement/movement.hpp>
#include <random>

std::mt19937 rng( std::time( NULL ) );

void antiaim::handle_direction( c_user_cmd *cmd ) {
    switch ( g_vars.exploits_antiaim_dir_type.value ) {
        case 0:
            dir = cmd->view_angles.y;
            break;
        case 1:
            dir = cmd->view_angles.y + 180.f;
            break;
        case 2:
            dir = cmd->view_angles.y + 90.f;
            break;
        case 3:
            dir = cmd->view_angles.y - 90.f;
            break;
        default:
            break;
    }

    dir = math::normalize( dir );
}

void antiaim::handle_fake( c_user_cmd *cmd ) {
    *globals::packet = true;
    std::uniform_real_distribution gen( -90.f, 90.f );

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

    if ( g_vars.exploits_antiaim_lby_break.value ) {
        const auto standing = glm::length( globals::local_player->velocity( ) ) < 1.0f;
        const auto air = ( globals::local_player->flags( ) & player_flags::on_ground ) || globals::local_player->velocity( ).z > 1.0f;

        if ( !g_interfaces.client_state->choked_commands( ) && ( g_interfaces.global_vars->curtime > g_animations.lower_body_realign_timer ) && ( standing || air ) )
            cmd->view_angles.y += g_vars.exploits_antiaim_lby_break_delta.value;
    }
    distortion( cmd );

    cmd->view_angles.y += g_vars.exploits_antiaim_yaw_offset.value;
    cmd->view_angles.y = math::normalize( cmd->view_angles.y );

    real.y = cmd->view_angles.y;
}

int antiaim::get_max_choke_ticks( ) {
    if ( g_config.get_hotkey( g_vars.misc_fake_walk_key, g_vars.misc_fake_walk_key_toggle.value ) )
        return g_vars.misc_fake_walk_value.value;

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
    const auto angle = ( float ) ( ( float ) ( 1.0f - std::powf( distortion_timer, 2 ) ) * g_vars.exploits_antiaim_distortion_range.value ) - ( float ) ( g_vars.exploits_antiaim_distortion_range.value * 0.5 );

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