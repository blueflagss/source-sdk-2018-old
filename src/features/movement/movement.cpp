#include "movement.hpp"
#include <features/prediction_manager/prediction_manager.hpp>

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
        fast_stop( cmd );

        if ( g_vars.misc_bunny_hop.value )
            cmd->buttons &= ~buttons::jump;

        if ( math::length_2d( g_prediction_context.velocity ) >= 30.0f ) {
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
    } else {
    }
}

void movement::quick_stop( c_user_cmd *cmd ) {
    if ( !globals::local_player )
        return;

    const auto anim_state = globals::local_player->anim_state( );

    if ( !anim_state )
        return;

    auto angle = math::vector_angle( globals::local_player->velocity( ) );

    float speed = glm::length( globals::local_player->velocity( ) );

    angle.y = globals::view_angles.y - angle.y;

    vector_3d direction;
    math::angle_vectors( angle, &direction );

    auto stop = direction * -speed;

    if ( anim_state->m_flVelocityLengthXY > 13.0f ) {
        cmd->forward_move = stop.x;
        cmd->side_move = stop.y;
    }

    else {
        cmd->forward_move = 0.0f;
        cmd->side_move = 0.0f;
    }
}

void movement::fast_stop( c_user_cmd *cmd ) {
    if ( !g_vars.misc_fast_stop.value )
        return;

    if ( globals::local_player )
        return;

    if ( glm::length( globals::local_player->velocity( ) ) < 5.0f )
        return;

    quick_stop( cmd );
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

void movement::slow( c_user_cmd *cmd, float wish_speed ) {
    //if ( !globals::local_player )
    //    return;

    //int i;
    //float addspeed, accelspeed, currentspeed;
    //vector_3d forward, right, up;
    //vector_3d wish_vel;

    //math::angle_vectors( globals::view_angles, &forward, &right, &up );

    //wish_vel = {
    //        math::normalize_angle( forward ).x * cmd->forward_move + math::normalize_angle( right ).x * cmd->side_move,
    //        math::normalize_angle( forward ).y * cmd->forward_move + math::normalize_angle( right ).y * cmd->side_move,
    //        0.0f };

    //currentspeed = glm::dot( globals::local_player->velocity( ), wish_vel );

    //addspeed = wish_speed - currentspeed;

    //if ( addspeed <= 0 )
    //    return;

    //accelspeed = addspeed;

    //if ( accelspeed > addspeed )
    //    accelspeed = addspeed;

    //float fRatio = accelspeed / ( globals::local_player->scoped( ) ? globals::local_weapon_data->max_speed_alt : globals::local_weapon_data->max_speed );

    //cmd->forward_move = -( globals::cvars::cl_forwardspeed->get_float( ) * fRatio );
    //cmd->side_move = -( globals::cvars::cl_sidespeed->get_float( ) * fRatio );
    //cmd->up_move = 0;

    //cmd->buttons &= ~buttons::walk;
    //cmd->buttons |= buttons::speed;
}

void movement::correct_movement( c_user_cmd *cmd, const vector_3d &wish_direction ) {
    if ( !( globals::local_player->flags( ) & FL_ONGROUND ) && cmd->view_angles.z != 0.f )
        cmd->side_move = 0.f;

    auto move_dir = vector_3d( cmd->forward_move, cmd->side_move, 0.f );
    auto len = math::normalize_place( move_dir );

    if ( !len )
        return;

    auto move_angle = math::vector_angle( move_dir );
    auto delta = cmd->view_angles.y - wish_direction.y;

    move_angle.y += delta;

    vector_3d dir = math::angle_vectors( move_angle );

    dir *= len;

    if ( globals::local_player->move_type( ) == move_types::ladder ) {
        if ( cmd->view_angles.x >= 45.f && wish_direction.x < 45.f && std::abs( delta ) <= 65.f )
            dir.x = -dir.x;

        cmd->forward_move = dir.x;
        cmd->side_move = dir.y;
    }
    
    else {
        if ( cmd->view_angles.x < -90.f || cmd->view_angles.x > 90.f )
            dir.x = -dir.x;

        cmd->forward_move = dir.x;
        cmd->side_move = dir.y;
    }
}