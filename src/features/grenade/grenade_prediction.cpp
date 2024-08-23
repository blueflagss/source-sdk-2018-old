#include "grenade_prediction.hpp"
#include <features/animations/animation_state.hpp>
#include <features/penetration/penetration.hpp>

int grenade_trajectory::physics_clip_velocity( const vector_3d &in, const vector_3d &normal, vector_3d &out, float overbounce ) {
    static const auto STOP_EPSILON = 0.1f;
    auto blocked = 0;

    const auto angle = normal[ 2 ];

    if ( angle > 0 )
        blocked |= 1;

    if ( !angle )
        blocked |= 2;

    const auto backoff = glm::dot( in, normal ) * overbounce;
    for ( auto i = 0; i < 3; i++ ) {
        const auto change = normal[ i ] * backoff;
        out[ i ] = in[ i ] - change;
        if ( out[ i ] > -STOP_EPSILON && out[ i ] < STOP_EPSILON )
            out[ i ] = 0;
    }

    return blocked;
}

void grenade_trajectory::push_entity( c_game_trace &trace, c_base_entity *player, const vector_3d &src, vector_3d &angle ) {
    c_trace_filter_simple filter( player, COLLISION_GROUP_PROJECTILE );

    vector_3d mins( -2.f, -2.f, -2.f );
    vector_3d maxs( 2.f, 2.f, 2.f );

    ray_t ray;

    auto end = src + angle;
    ray.init( src, end, mins, maxs );

    g_interfaces.engine_trace->trace_ray( ray, 0x200400B, &filter, &trace );
}

void grenade_trajectory::resolve_fly_collision_custom( c_game_trace &trace, vector_3d &vec_velocity, float interval ) {
    if ( trace.entity && g_penetration.is_breakable_entity( trace.entity ) ) {
        if ( !trace.entity->is( HASH_CT( "CFuncBrush" ) ) &&
             !trace.entity->is( HASH_CT( "CBaseDoor" ) ) &&
             !trace.entity->is( HASH_CT( "CCSPlayer" ) ) &&
             !trace.entity->is( HASH_CT( "CBaseEntity" ) ) ) {
            push_entity( trace, trace.entity->get< c_base_entity * >( ), trace.end_pos, vec_velocity );
            vel *= 0.4f;
            return;
        }
    }

    const auto fl_surface_elasticity = 1.f;
    const auto fl_grenade_elasticity = 0.45f;

    auto fl_total_elasticity = fl_grenade_elasticity * fl_surface_elasticity;

    if ( fl_total_elasticity > 0.9f )
        fl_total_elasticity = 0.9f;

    if ( fl_total_elasticity < 0.0f )
        fl_total_elasticity = 0.0f;

    vector_3d vec_abs_velocity;
    physics_clip_velocity( vel, trace.plane.normal, vec_abs_velocity, 2.0f );
    vec_abs_velocity *= fl_total_elasticity;

    if ( trace.plane.normal.z > 0.7f ) {
        float speed = math::length_sqr( vel );

        if ( speed > 96000.f ) {
            float len = glm::dot( math::normalize_angle( vel ), trace.plane.normal );

            if ( len > 0.5f )
                vel *= 1.5f - len;
        }

        if ( speed < 400.f )
            vel = vector_3d( );

        else {
            vel = vec_abs_velocity;
            vec_abs_velocity *= ( ( 1.0f - trace.fraction ) * interval );

            push_entity( trace, globals::local_player, trace.end_pos, vec_abs_velocity );
        }
    } else {
        vel = vec_abs_velocity;
        vec_abs_velocity *= ( ( 1.0f - trace.fraction ) * interval );

        push_entity( trace, globals::local_player, trace.end_pos, vec_abs_velocity );
    }

    bounces.push_back( std::make_pair( trace.end_pos, false ) );
}

void grenade_trajectory::on_grenade_thrown( event_t *evt ) {
    if ( !globals::local_player || !evt )
        return;

    auto userid = g_interfaces.engine_client->get_player_for_user_id( evt->get_int( _xs( "userid" ) ) );

    if ( userid != globals::local_player->index( ) )
        return;

    if ( saved_path.empty( ) )
        return;

    current_path = saved_path;
    spawn_time = g_interfaces.global_vars->curtime;
}

void grenade_trajectory::render_path( ) {
    if ( bounces.empty( ) || path.empty( ) )
        return;

    if ( globals::local_weapon->is_grenade( ) ) {
        last_weapon_id = globals::local_weapon->item_definition_index( );

        if ( path.size( ) > 2 ) {
            auto prev = std::get< 0 >( path.front( ) );

            for ( const auto &it : path ) {
                vector_2d nade_end;
                vector_2d nade_start;

                if ( render::world_to_screen( prev, nade_start ) && render::world_to_screen( std::get< 0 >( it ), nade_end ) )
                    render::line( nade_start, nade_end, color::white( ), 2.0f );

                prev = std::get< 0 >( it );
            }
        }

        if ( bounces.size( ) > 1 )
            bounces.back( ).second = true;

        if ( bounces.size( ) > 1 ) {
            for ( const auto &bounce : bounces ) {
                vector_2d bounce_screen;

                if ( render::world_to_screen( bounce.first, bounce_screen ) ) {
                    auto radius = 3;

                    bounce_screen.x -= radius - 3.15f;
                    bounce_screen.y -= radius - 2.55f;

                    render::circle_filled( bounce_screen, radius, 16, bounce.second ? color( 0, 255, 0 ) : color( 255, 0, 0 ) );
                    render::circle( bounce_screen, radius, 16, color( 0, 0, 0 ), 1.5f );
                }
            }
        }
    }
}

void grenade_trajectory::simulate_path( ) {
    if ( !globals::local_player || !globals::local_weapon )
        return;

    if ( !globals::local_weapon->is_grenade( ) ) {
        path.clear( );
        return;
    }

    auto src = globals::local_player->get_shoot_position( );
    auto ang_throw = globals::view_angles;
    auto pitch = ang_throw.x;

    if ( pitch < -90 )
        pitch += 360.f;
    else if ( pitch > 90.f )
        pitch -= 360.f;

    ang_throw.x = pitch - ( 90.f - std::fabs( pitch ) ) * 10.f / 90.f;

    const auto grenade = reinterpret_cast< c_base_cs_grenade * >( globals::local_weapon );

    auto throw_velocity = std::clamp( globals::local_weapon_data->throw_velocity * 0.9f, 15.f, 750.f );
    throw_velocity *= grenade->throw_strength( ) * 0.7f + 0.3f;

    vector_3d forward;
    math::angle_vectors( ang_throw, &forward );
    src.z += grenade->throw_strength( ) * 12.f - 12.f;

    c_game_trace trace;
    c_trace_filter_simple filter( globals::local_player, COLLISION_GROUP_PROJECTILE );

    vector_3d mins( -2.f, -2.f, -2.f );
    vector_3d maxs( 2.f, 2.f, 2.f );

    ray_t ray;
    ray.init( src, src + forward * 22.f, mins, maxs );

    g_interfaces.engine_trace->trace_ray( ray, mask_solid, &filter, &trace );
    src = trace.end_pos - ( forward * 6.f );

    ang_throw = globals::local_player->abs_velocity( );
    ang_throw *= 1.25f;
    ang_throw += ( forward * throw_velocity );

    vel = ang_throw;

    const auto interval = g_interfaces.global_vars->interval_per_tick;
    const auto log_step = game::time_to_ticks( 0.04f );

    log_timer = 0;

    bounces.clear( );
    path.clear( );

    for ( unsigned int i = 0; i < 4096u; ++i ) {
        if ( !log_timer ) {
            vector_3d angles = math::vector_angle( src );

            path.push_back( std::make_tuple( src, did_collide, angles ) );
            did_collide = false;
        }

        const auto s = tick( globals::local_player, globals::local_weapon, src, i, interval );

        if ( s & 1 )
            break;

        if ( s & 2 || log_timer >= log_step )
            log_timer = 0;
        else
            ++log_timer;

        if ( s & 2 )
            did_collide = true;

        if ( vel == vector_3d( ) )
            break;
    }

    path.push_back( std::make_tuple( src, false, vector_3d( ) ) );
    bounces.push_back( std::make_pair( src, false ) );

    saved_path = path;
}

void grenade_trajectory::physics_add_gravity_move( c_base_entity *player, vector_3d &move ) {
    const auto gravity = globals::cvars::sv_gravity->get_float( ) * 0.4f;

    move.x = vel.x * g_interfaces.global_vars->interval_per_tick;
    move.y = vel.y * g_interfaces.global_vars->interval_per_tick;

    float z = vel.z - ( gravity * g_interfaces.global_vars->interval_per_tick );

    move.z = ( ( vel.z + z ) / 2.f ) * g_interfaces.global_vars->interval_per_tick;

    vel.z = z;
}

bool grenade_trajectory::check_detonate( c_cs_weapon_base *weapon, const c_game_trace &tr, int tick, float interval ) {
    if ( !weapon )
        return false;

    float time = game::ticks_to_time( tick );

    switch ( weapon->item_definition_index( ) ) {
        case weapons::flashbang:
        case weapons::hegrenade:
            return time >= 1.5f && !( tick % game::time_to_ticks( 0.2f ) );

        case weapons::smoke:
            return glm::length( vel ) <= 0.1f && !( tick % game::time_to_ticks( 0.2f ) );

        case weapons::decoy:
            return glm::length( vel ) <= 0.2f && !( tick % game::time_to_ticks( 0.2f ) );

        case weapons::molotov:
        case weapons::firebomb:
            if ( tr.fraction != 1.f && ( std::cos( math::deg_to_rad( globals::cvars::weapon_molotov_maxdetonateslope->get_float( ) ) ) <= tr.plane.normal.z ) )
                return true;

            return time >= globals::cvars::molotov_throw_detonate_time->get_float( ) && !( tick % game::time_to_ticks( 0.1f ) );

        default:
            return false;
    }

    return false;
}

int grenade_trajectory::tick( c_base_entity *ent, c_cs_weapon_base *weapon, vector_3d &src, int tick, float interval ) {
    c_game_trace trace;
    vector_3d move;

    physics_add_gravity_move( ent, move );
    push_entity( trace, ent, src, move );

    auto result = 0;

    if ( check_detonate( weapon, trace, tick, interval ) ) {
        result |= 1;
    }

    if ( trace.fraction != 1.0f ) {
        result |= 2;

        resolve_fly_collision_custom( trace, move, interval );
    }

    src = trace.end_pos;

    return result;
}