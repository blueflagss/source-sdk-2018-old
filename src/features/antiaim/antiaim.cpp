#include "antiaim.hpp"

#include "features/movement/movement.hpp"

#include <random>

std::mt19937 rng( time( NULL ) );

void antiaim::handle_direction( c_user_cmd *cmd ) {
    switch ( g_vars.exploits_antiaim_dir_type.value ) {
        case 0:
            dir = cmd->view_angles.y;
            break;
        case 1:// backward
            dir = cmd->view_angles.y + 180.f;
            break;
        case 2:// left
            dir = cmd->view_angles.y + 90.f;
            break;
        case 3:// right
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

    cmd->view_angles.y = math::normalize( cmd->view_angles.y );
    real.y = cmd->view_angles.y;
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
        if ( *globals::packet && globals::old_packet )
            *globals::packet = false;

        if ( !*globals::packet )
            handle_real( cmd );
        else
            handle_fake( cmd );
    } else
        handle_real( cmd );
}
