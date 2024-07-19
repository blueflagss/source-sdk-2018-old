#include "grenade_prediction.hpp"

int grenade_prediction::physics_clip_velocity( const vector_3d &in, const vector_3d &normal, vector_3d &out, float overbounce ) {
    static const auto STOP_EPSILON = 0.1f;
    auto blocked = 0;

    const auto angle = normal[ 2 ];
    if ( angle > 0 )
        blocked |= 1;// floor
    if ( !angle )
        blocked |= 2;// step

    const auto backoff = glm::dot( in, normal ) * overbounce;
    for ( auto i = 0; i < 3; i++ ) {
        const auto change = normal[ i ] * backoff;
        out[ i ] = in[ i ] - change;
        if ( out[ i ] > -STOP_EPSILON && out[ i ] < STOP_EPSILON )
            out[ i ] = 0;
    }

    return blocked;
}

void grenade_prediction::push_entity( c_cs_player* player, vector_3d& angle ) {
    c_game_trace trace;
    c_trace_filter_hitscan filter;
    filter.player = globals::local_player;

    vector_3d mins( -2.f, -2.f, -2.f );
    vector_3d maxs( 2.f, 2.f, 2.f );

    ray_t ray;

    auto src = trace.end_pos;

    ray.init( src, angle, mins, maxs );

    g_interfaces.engine_trace->trace_ray( ray, 0x46004003, &filter, &trace );

    angle = trace.end_pos;
}

void grenade_prediction::ResolveFlyCollisionCustom( c_game_trace &trace, vector_3d &vecVelocity, float interval ) {
    const auto fl_surface_elasticity = 1.f;
    const auto fl_grenade_elasticity = 0.45f;

    auto fl_total_elasticity = fl_grenade_elasticity * fl_surface_elasticity;

    if ( fl_total_elasticity > 0.9f )
        fl_total_elasticity = 0.9f;

    if ( fl_total_elasticity < 0.0f )
        fl_total_elasticity = 0.0f;

    vector_3d vec_abs_velocity = globals::local_player->velocity( );
    physics_clip_velocity( vecVelocity, trace.plane.normal, vec_abs_velocity, 2.0f );
    vec_abs_velocity *= fl_total_elasticity;

    {


            vec_abs_velocity *= ( ( 1.0f - trace.fraction ) * interval );

            push_entity( globals::local_player, vec_abs_velocity );
        

    }
}

void grenade_prediction::render( ) {
    if ( !g_interfaces.engine_client->is_in_game( ) || !g_interfaces.engine_client->is_connected( ) )
        return;

    if ( !globals::local_player || !globals::local_player->alive( ) )
        return;

    if ( !globals::local_weapon->is_grenade( ) )
        return;

    if ( path.empty( ) )
        return;

    if ( path.size( ) > 1 ) {
        vector_2d nade_end;
        vector_2d nade_start;

        auto prev = std::get< 0 >( path.front( ) );

        for ( const auto &it : path ) {
            c_game_trace tr;
            c_trace_filter_world_and_props filter;

            ray_t ray;

            ray.init( std::get< 0 >( it ), globals::local_player->get_shoot_position( ) );
            g_interfaces.engine_trace->trace_ray( ray, mask_shot, &filter, &tr );

                if ( render::world_to_screen( prev, nade_start ) && render::world_to_screen( std::get< 0 >( it ), nade_end ) )
                    render::line( nade_start, nade_end, color::white( ), 1.0f );
            

            prev = std::get< 0 >( it );
        }
    }
}

void grenade_prediction::simulate_path( c_cs_player *player, vector_3d &vecSrc ) {
    if ( !player )
        return;

    const auto weapon = reinterpret_cast< c_base_cs_grenade * >( globals::local_weapon );

    if ( !weapon )
        return;

    vector_3d angThrow = globals::view_angles;

    if ( angThrow.x < -90 )
        angThrow.x += 360.f;
    else if ( angThrow.x > 90.f )
        angThrow.x -= 360.f;

    angThrow.x -= ( 90.f - abs( angThrow.x ) ) * 10.f / 90.f;

    auto ThrowStrength = std::clamp( weapon->throw_strength( ), 0.f, 1.f );
    auto flVel = std::clamp( globals::local_weapon_data->throw_velocity * 0.9f, 15.f, 750.f );

    flVel *= ThrowStrength * 0.7f + 0.3f;

    vector_3d vForward;
    math::angle_vectors( angThrow, &vForward );

    vecSrc.z += ThrowStrength * 12.f - 12.f;

    c_game_trace trace;
    c_trace_filter_world_and_props filter;

    vector_3d mins( -2.f, -2.f, -2.f );
    vector_3d maxs( 2.f, 2.f, 2.f );

    ray_t ray;
    ray.init( vecSrc, vecSrc + vForward * 22.f, mins, maxs );

    g_interfaces.engine_trace->trace_ray( ray, mask_solid | contents_current_90, &filter, &trace );

    vecSrc = trace.end_pos - vForward * 6.f;

    auto vecThrow = vForward * flVel + globals::local_player->abs_velocity( ) * 1.25f;

    const auto interval = g_interfaces.global_vars->interval_per_tick;
    const auto logstep = static_cast< int >( 0.04f / interval );

    log_timer = 0;

    path.clear( );

    for ( unsigned int i = 0; i < 128; ++i ) {
        if ( !log_timer ) {
            vector_3d angles = math::vector_angle( vecSrc );

            path.push_back( std::make_tuple( vecSrc, did_collide, angles ) );
            did_collide = false;
        }

        const auto s = tick( globals::local_player, vecSrc, vecThrow, i, interval );

        if ( s & 1 )
            break;

        if ( s & 2 || log_timer >= logstep )
            log_timer = 0;
        else
            ++log_timer;

        if ( s & 2 )
            did_collide = true;
    }

    path.push_back( std::make_tuple( vecSrc, false, vector_3d( ) ) );
}

void grenade_prediction::PhysicsAddGravityMove( c_cs_player *player, vector_3d &move ) {
    vector_3d vecAbsVelocity = player->abs_velocity( );

    move.x = vecAbsVelocity.x * g_interfaces.global_vars->frametime;
    move.y = vecAbsVelocity.y * g_interfaces.global_vars->frametime;

    if ( player->flags( ) & player_flags::on_ground ) {
        move.z = vecAbsVelocity.z * g_interfaces.global_vars->frametime;
        return;
    }

    // linear acceleration due to gravity
    float newZVelocity = vecAbsVelocity.z - globals::cvars::sv_gravity->get_float( ) * g_interfaces.global_vars->frametime;

    move.z = ( ( vecAbsVelocity.z + newZVelocity ) / 2.0 ) * g_interfaces.global_vars->frametime;

    vecAbsVelocity.z = newZVelocity;
}

bool grenade_prediction::check_detonate( const vector_3d &vecThrow, const c_game_trace &tr, int tick, float interval ) {
    fire = false;

    switch ( globals::local_weapon->item_definition_index( ) ) {
        case weapons::smoke:
        case weapons::decoy:
            if ( math::length_2d( vecThrow ) < 0.1f ) {
                const auto det_tick_mod = static_cast< int >( 0.2f / interval );
                return !( tick % det_tick_mod );
            }
            return false;
        case weapons::molotov:
        case weapons::firebomb:
            if ( tr.fraction != 1.0f && tr.plane.normal.z > 0.7f )
                return true;
        case weapons::flashbang:
        case weapons::hegrenade:
            fire = static_cast< float >( tick ) * interval > 1.5f && !( tick % static_cast< int >( 0.2f / interval ) );
            return fire;
        default: return false;
    }

    return true;
}
int grenade_prediction::tick( c_cs_player *player, vector_3d &src, vector_3d &vec_throw, int tick, float interval ) {
    c_game_trace trace;
    vector_3d move;

    PhysicsAddGravityMove( player, move );
    push_entity( player, move );

    auto result = 0;

    if ( check_detonate( vec_throw, trace, tick, interval ) )
        result |= 1;

    if ( trace.fraction != 1.0f ) {
        result |= 2;

        ResolveFlyCollisionCustom( trace, move, interval );
    }

    return result;
}
