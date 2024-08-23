#include "grenade_warning.hpp"
#include <features/animations/animation_state.hpp>
#include <features/penetration/penetration.hpp>

int grenade_warning::physics_clip_velocity( const vector_3d &in, const vector_3d &normal, vector_3d &out, float overbounce ) {
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

void grenade_warning::push_entity( c_game_trace &trace, c_base_entity *player, const vector_3d &src, vector_3d &angle ) {
    c_trace_filter_simple filter( player, COLLISION_GROUP_INTERACTIVE_DEBRIS );

    vector_3d mins( -2.f, -2.f, -2.f );
    vector_3d maxs( 2.f, 2.f, 2.f );

    ray_t ray;

    auto end = src + angle;
    ray.init( src, end, mins, maxs );

    g_interfaces.engine_trace->trace_ray( ray, 0x200400B, &filter, &trace );
}

void grenade_warning::resolve_fly_collision_custom( c_game_trace &trace, vector_3d &vec_velocity, float interval ) {
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

    if ( bounces > 20 )
        detonated = true;
    else
        bounces++;
}

void grenade_warning::on_grenade_thrown( event_t *evt ) {
    if ( !globals::local_player || !evt )
        return;

    auto userid = g_interfaces.engine_client->get_player_for_user_id( evt->get_int( _xs( "userid" ) ) );

    if ( userid != globals::local_player->index( ) )
        return;

    g_nade_events.push_back( nade_event_t( userid, g_interfaces.global_vars->curtime ) );
}

void grenade_warning::render_path( ) {
    if ( path.empty( ) )
        return;

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
}

void grenade_warning::simulate_path( ) {
    if ( !this->entity )
        return;

    const auto grenade = this->entity->get< c_base_cs_grenade * >( );
    const auto interval = g_interfaces.global_vars->interval_per_tick;
    const auto log_step = game::time_to_ticks( 0.04f );

    log_timer = 0;

    path.clear( );

    auto thrower_entity = g_interfaces.entity_list->get_client_entity_from_handle< c_cs_player * >( grenade->thrower( ) );
    auto spawn_time = grenade->spawn_time( );

    for ( auto a : g_nade_events ) {
        if ( a.index == thrower_entity->index( ) ) {
            spawn_time = a.time;
        }
    }

    this->spawn_time = spawn_time;
    this->curtime = g_interfaces.global_vars->curtime - spawn_time;
    this->bounces = 0;
    detonated = false;

    auto src = this->origin = this->entity->origin( );

    for ( unsigned int i = 0; i < 1024u; ++i ) {
        if ( !log_timer ) {
            path.push_back( std::make_tuple( src, did_collide ) );
            did_collide = false;
        }

        const auto s = tick( this->entity, src, i, interval );

        if ( s & 1 || detonated )
            break;

        if ( s & 2 || log_timer >= log_step )
            log_timer = 0;
        else
            ++log_timer;

        if ( s & 2 )
            did_collide = true;

        this->curtime += game::ticks_to_time( i );

        if ( vel == vector_3d( ) )
            break;
    }

    path.push_back( std::make_tuple( src, false ) );
}

void grenade_warning::physics_add_gravity_move( c_base_entity *player, vector_3d &move ) {
    const auto gravity = globals::cvars::sv_gravity->get_float( ) * 0.4f;

    move.x = vel.x * g_interfaces.global_vars->interval_per_tick;
    move.y = vel.y * g_interfaces.global_vars->interval_per_tick;

    float z = vel.z - ( gravity * g_interfaces.global_vars->interval_per_tick );

    move.z = ( ( vel.z + z ) / 2.f ) * g_interfaces.global_vars->interval_per_tick;

    vel.z = z;
}

bool grenade_warning::check_detonate( c_cs_weapon_base *weapon, const c_game_trace &tr, int tick, float interval ) {
    if ( !weapon )
        return false;

    if ( next_think_tick < tick )
        return false;

    switch ( this->weapon_id ) {
        case weapons::smoke:
            set_next_think_tick( 1.5f );
            break;
        case weapons::decoy:
            set_next_think_tick( 2.f );
            break;
        case weapons::flashbang:
        case weapons::hegrenade:
            detonate_time = 1.5f;
            set_next_think_tick( 0.02f );
            break;
        case weapons::molotov:
        case weapons::firebomb:
            detonate_time = globals::cvars::molotov_throw_detonate_time->get_float( );
            set_next_think_tick( 0.02f );
            break;
    }

    if ( this->curtime >= detonate_time )
        return true;

    return false;
}

void grenade_warning::set_next_think_tick( float think_time ) {
    next_think_tick = game::time_to_ticks( think_time );
}

int grenade_warning::tick( c_base_entity *ent, vector_3d &src, int tick, float interval ) {
    c_game_trace trace;
    vector_3d move;

    auto result = 0;

    if ( check_detonate( ent->get< c_base_cs_grenade * >( ), trace, tick, interval ) )
        detonated = true;

    if ( detonated )
        result |= 1;

    if ( detonated )
        return result;

    physics_add_gravity_move( ent, move );
    push_entity( trace, ent, src, move );

    if ( trace.fraction != 1.0f ) {
        result |= 2;

        resolve_fly_collision_custom( trace, move, interval );
    }

    src = trace.end_pos;

    return result;
}