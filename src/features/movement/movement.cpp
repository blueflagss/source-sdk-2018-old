#include "movement.hpp"
#include <features/engine_prediction/engine_prediction.hpp>

void movement::on_create_move( c_user_cmd *cmd, const vector_3d &old_angles ) {
    if ( !globals::local_player )
        return;

    if ( !globals::local_player->alive( ) )
        return;

    if ( globals::local_player->move_type( ) == move_types::ladder || globals::local_player->move_type( ) == move_types::noclip )
        return;

    ground_ticks = 0;

    if ( globals::local_player->flags( ) & player_flags::on_ground ) {
        if ( ground_ticks <= 3 )
            ground_ticks++;
    } 
    
    else {
        ground_ticks = 0;
    }

    if ( !( globals::local_player->flags( ) & player_flags::on_ground ) ) {
        if ( g_vars.misc_bunny_hop.value )
                cmd->buttons &= ~buttons::jump;

        if ( math::length_2d( g_prediction.predicted_velocity ) >= 30.0f ) {
            switch ( g_vars.misc_auto_strafe_type.value ) {
                case strafe_type::view_angles: {
                    if ( std::abs( cmd->mousedx ) > 2 ) {
                        cmd->side_move = cmd->mousedx < 0 ? -globals::cvars::cl_sidespeed->get_float( ) : globals::cvars::cl_sidespeed->get_float( );
                    } else {
                        cmd->side_move = cmd->command_number % 2 ? -globals::cvars::cl_sidespeed->get_float( ) : globals::cvars::cl_sidespeed->get_float( );
                        cmd->forward_move = globals::cvars::cl_forwardspeed->get_float( );
                    }

                } break;
                case strafe_type::movement_keys: {
                    directional_strafe( cmd, old_angles );
                } break;
            }
        }
    }
}

void movement::directional_strafe( c_user_cmd *cmd, const vector_3d &old_angles ) {
    static int strafer_flags = 0;

    if ( !!( globals::local_player->flags( ) & player_flags::on_ground ) ) {
        strafer_flags = 0;
        return;
    }

    auto velocity = globals::local_player->velocity( );
    auto velocity_len = math::length_2d( velocity );

    if ( velocity_len <= 0.0f ) {
        strafer_flags = 0;
        return;
    }

    auto ideal_step = std::min< float >( 90.0f, 845.5f / velocity_len );
    auto velocity_yaw = ( velocity.y || velocity.x ) ? math::rad_to_deg( atan2f( velocity.y, velocity.x ) ) : 0.0f;

    auto unmod_angles = old_angles;
    auto angles = old_angles;

    if ( velocity_len < 2.0f && !!( cmd->buttons & buttons::jump ) )
        cmd->forward_move = 450.0f;

    auto forward_move = cmd->forward_move;
    auto on_ground = !!( globals::local_player->flags( ) & player_flags::on_ground );

    if ( forward_move || cmd->side_move ) {
        cmd->forward_move = 0.0f;

        if ( velocity_len != 0.0f && std::abs( velocity.z ) != 0.0f ) {
            if ( !on_ground ) {
            DO_IT_AGAIN:
                vector_3d fwd;
                math::angle_vectors( angles, &fwd );

                auto right = glm::cross( fwd, vector_3d( 0.0f, 0.0f, 1.0f ) );

                auto v262 = ( fwd.x * forward_move ) + ( cmd->side_move * right.x );
                auto v263 = ( right.y * cmd->side_move ) + ( fwd.y * forward_move );

                angles.y = ( v262 || v263 ) ? math::rad_to_deg( std::atan2f( v263, v262 ) ) : 0.0f;
            }
        }
    }

    auto yaw_to_use = 0.0f;

    strafer_flags &= ~4;

    if ( !on_ground ) {
        auto clamped_angles = angles.y;

        if ( clamped_angles < -180.0f ) clamped_angles += 360.0f;
        if ( clamped_angles > 180.0f ) clamped_angles -= 360.0f;

        yaw_to_use = old_angles.y;

        strafer_flags |= 4;
    }

    if ( strafer_flags & 4 ) {
        auto diff = angles.y - yaw_to_use;

        if ( diff < -180.0f ) diff += 360.0f;
        if ( diff > 180.0f ) diff -= 360.0f;

        if ( std::abs( diff ) > ideal_step && std::abs( diff ) <= 30.0f ) {
            auto move = 450.0f;

            if ( diff < 0.0f )
                move *= -1.0f;

            cmd->side_move = move;
            return;
        }
    }

    auto diff = angles.y - velocity_yaw;

    if ( diff < -180.0f ) diff += 360.0f;
    if ( diff > 180.0f ) diff -= 360.0f;

    auto step = 0.6f * ( ideal_step + ideal_step );
    auto side_move = 0.0f;

    if ( std::abs( diff ) > 170.0f && velocity_len > 80.0f || diff > step && velocity_len > 80.0f ) {
        angles.y = step + velocity_yaw;
        cmd->side_move = -450.0f;
    } else if ( -step <= diff || velocity_len <= 80.0f ) {
        if ( strafer_flags & 1 ) {
            angles.y -= ideal_step;
            cmd->side_move = -450.0f;
        } else {
            angles.y += ideal_step;
            cmd->side_move = 450.0f;
        }
    } else {
        angles.y = velocity_yaw - step;
        cmd->side_move = 450.0f;
    }

    if ( !( cmd->buttons & buttons::back ) && !cmd->side_move )
        goto DO_IT_AGAIN;

    strafer_flags ^= ( strafer_flags ^ ~strafer_flags ) & 1;

    if ( angles.y < -180.0f ) angles.y += 360.0f;
    if ( angles.y > 180.0f ) angles.y -= 360.0f;

    correct_movement( cmd, angles );
}

void movement::correct_movement( c_user_cmd *cmd, const vector_3d &wish_direction ) {
    vector_3d view_fwd, view_right, view_up, cmd_fwd, cmd_right, cmd_up;
    auto view_angles = cmd->view_angles;

    math::angle_vectors( wish_direction, &view_fwd, &view_right, &view_up );
    math::angle_vectors( view_angles, &cmd_fwd, &cmd_right, &cmd_up );

    const auto v12 = sqrt( view_up.z * view_up.z );
    const auto v10 = math::length_2d( view_right );
    const auto v8 = math::length_2d( view_fwd );

    const auto norm_view_fwd = vector_3d( 1.0f / v8 * view_fwd.x, 1.0f / v8 * view_fwd.y, 0.0f );
    const auto norm_view_right = vector_3d( 1.0f / v10 * view_right.x, 1.0f / v10 * view_right.y, 0.0f );
    const auto norm_view_up = vector_3d( 0.0f, 0.0f, 1.0f / v12 * view_up.z );

    const auto v14 = math::length_2d( view_fwd );
    const auto v16 = math::length_2d( view_right );
    const auto v18 = std::sqrt( cmd_up.z * cmd_up.z );

    const auto norm_cmd_fwd = vector_3d( 1.0f / v14 * cmd_fwd.x, 1.0f / v14 * cmd_fwd.y, 0.0f );
    const auto norm_cmd_right = vector_3d( 1.0f / v16 * cmd_right.x, 1.0f / v16 * cmd_right.y, 0.0f );
    const auto norm_cmd_up = vector_3d( 0.0f, 0.0f, 1.0f / v18 * cmd_up.z );

    const float v22 = norm_view_fwd.x * cmd->forward_move;
    const float v26 = norm_view_fwd.y * cmd->forward_move;
    const float v28 = norm_view_fwd.z * cmd->forward_move;
    const float v24 = norm_view_right.x * cmd->side_move;
    const float v23 = norm_view_right.y * cmd->side_move;
    const float v25 = norm_view_right.z * cmd->side_move;
    const float v30 = norm_view_up.x * cmd->up_move;
    const float v27 = norm_view_up.z * cmd->up_move;
    const float v29 = norm_view_up.y * cmd->up_move;

    cmd->forward_move = norm_cmd_fwd.x * v24 + norm_cmd_fwd.y * v23 + norm_cmd_fwd.z * v25 + ( norm_cmd_fwd.x * v22 + norm_cmd_fwd.y * v26 + norm_cmd_fwd.z * v28 ) + ( norm_cmd_fwd.y * v30 + norm_cmd_fwd.x * v29 + norm_cmd_fwd.z * v27 );
    cmd->side_move = norm_cmd_right.x * v24 + norm_cmd_right.y * v23 + norm_cmd_right.z * v25 + ( norm_cmd_right.x * v22 + norm_cmd_right.y * v26 + norm_cmd_right.z * v28 ) + ( norm_cmd_right.x * v29 + norm_cmd_right.y * v30 + norm_cmd_right.z * v27 );
    cmd->up_move = norm_cmd_up.x * v23 + norm_cmd_up.y * v24 + norm_cmd_up.z * v25 + ( norm_cmd_up.x * v26 + norm_cmd_up.y * v22 + norm_cmd_up.z * v28 ) + ( norm_cmd_up.x * v30 + norm_cmd_up.y * v29 + norm_cmd_up.z * v27 );

    cmd->forward_move = std::clamp( cmd->forward_move, -globals::cvars::cl_forwardspeed->get_float( ), globals::cvars::cl_forwardspeed->get_float( ) );
    cmd->side_move = std::clamp( cmd->side_move, -globals::cvars::cl_sidespeed->get_float( ), globals::cvars::cl_sidespeed->get_float( ) );
    cmd->up_move = std::clamp( cmd->up_move, -globals::cvars::cl_upspeed->get_float( ), globals::cvars::cl_upspeed->get_float( ) );
}
