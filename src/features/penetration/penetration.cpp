#include "penetration.hpp"
#include <features/animations/animation_sync.hpp>

enum {
    char_tex_antlion = 'A',
    char_tex_bloodyflesh = 'B',
    char_tex_concrete = 'C',
    char_tex_dirt = 'D',
    char_tex_eggshell = 'E',
    char_tex_flesh = 'F',
    char_tex_grate = 'G',
    char_tex_alienflesh = 'H',
    char_tex_clip = 'I',
    char_tex_grass = 'J',
    char_tex_snow = 'K',
    char_tex_plastic = 'L',
    char_tex_metal = 'M',
    char_tex_sand = 'N',
    char_tex_foliage = 'O',
    char_tex_computer = 'P',
    char_tex_asphalt = 'Q',
    char_tex_brick = 'R',
    char_tex_slosh = 'S',
    char_tex_tile = 'T',
    char_tex_cardboard = 'U',
    char_tex_vent = 'V',
    char_tex_wood = 'W',
    char_tex_glass = 'Y',
    char_tex_warpshield = 'Z'
};

c_fire_bullet_data penetration_system::run( const vector_3d src, const vector_3d end, c_cs_player *ent, const float &in_damage, const std::array< matrix_3x4, 128 > &bones, bool is_zeus, c_cs_weapon_info *info_override ) {
    c_fire_bullet_data info{ };

    if ( !ent )
        return { };

    if ( !globals::local_weapon )
        return { };

    const auto data = info_override == nullptr ? globals::local_weapon_data : info_override;

    if ( !data )
        return { };

    info.damage = data->damage;
    info.in_damage = in_damage;

    c_trace_filter_hitscan filter{ };

    vector_3d origin;

    bool result = simulate_fire_bullet( data, src, end, info, bones, is_zeus, ent );

    if ( !result || info.damage < 1.0f )
        return { };

    return info;
}

bool penetration_system::simulate_fire_bullet( const c_cs_weapon_info *data, vector_3d src, vector_3d pos, c_fire_bullet_data &fire_info, const std::array< matrix_3x4, 128 > &bones, bool is_zeus, c_cs_player *ent ) {
    vector_3d direction = math::normalize_angle( pos - src );

    fire_info.penetrate_count = 4;
    auto length = 0.f;

    c_game_trace enter_trace{ };

    fire_info.did_hit = false;

    static c_trace_filter_skip_two_entities filter;
    filter.skip1 = globals::local_player;
    filter.skip2 = nullptr;

    while ( fire_info.penetrate_count > 0 && fire_info.damage > 0.0f ) {
        const auto length_remaining = data->range - length;
        auto end = src + direction * length_remaining;

        ray_t r{ };
        r.init( src, end );
        g_interfaces.engine_trace->trace_ray( r, mask_shot, &filter, &enter_trace );

        if ( ent ) 
            clip_trace_to_player( ent, src, end + ( direction * 40.0f ), mask_shot, &filter, enter_trace );
        
        else
            UTIL_ClipTraceToPlayers( src, end + ( direction * 40.0f ), mask_shot, &filter, &enter_trace, -60.0f );

        if ( enter_trace.fraction == 1.f )
            return false;

        length += enter_trace.fraction * length_remaining;
        fire_info.damage *= std::powf( data->range_modifier, length * 0.002f );

        if ( ( enter_trace.entity && enter_trace.entity == ent ) || ( ( enter_trace.hit_group >= hitgroup_head && enter_trace.hit_group <= hitgroup_rightleg ) || enter_trace.hit_group == hitgroup_gear ) ) {
            auto scaled_damage = scale_damage( ent, fire_info.damage, data->armor_ratio, enter_trace.hit_group, is_zeus );

            fire_info.did_hit = true;
            fire_info.bullet_end = enter_trace.end_pos;
            fire_info.impacts[ fire_info.impact_count++ ] = enter_trace.end_pos;
            fire_info.out_damage = scaled_damage;
            fire_info.out_hitgroup = enter_trace.hit_group;

            return true;
        }

        fire_info.impacts[ fire_info.impact_count++ ] = enter_trace.end_pos;

        const auto enter_surf = g_interfaces.physics_props->get_surface_data( enter_trace.surface.surface_props );
        if ( ( length > 3000.f && data->penetration > 0.f ) || ( !enter_surf || enter_surf->game.penetration_modifier < 0.1f ) )
            fire_info.penetrate_count = 0;

        if ( !handle_bullet_penetration( data, enter_trace, src, direction, fire_info.penetrate_count, fire_info.damage, data->penetration ) ) {
            fire_info.damage = 0.f;
            break;
        }
    }

    if ( !fire_info.did_hit )
        fire_info.out_damage = fire_info.damage;
    else {
        return true;
    }
}

bool penetration_system::is_breakable_entity( c_base_entity *ent ) {
    static auto _is_breakable = signature::find( _xs( "client.dll" ), _xs( "E8 ? ? ? ? 84 C0 75 A1" ) ).add( 0x1 ).rel32( );
    static auto takedamage_offset = _is_breakable.add( 38 ).deref( ).get< std::uint32_t >( );

    if ( !ent || !ent->index( ) )
        return false;

    auto &takedmg = *reinterpret_cast< std::uint8_t * >( reinterpret_cast< std::uintptr_t >( ent ) + takedamage_offset );

    const auto backup_takedmg = takedmg;
    auto cc = ent->get_client_class( );

    if ( cc ) {
        const auto name = HASH( cc->network_name );

        if ( name == HASH_CT( "CBreakableSurface" ) )
            takedmg = 2;

        else if ( name == HASH_CT( "CBaseDoor" ) || name == HASH_CT( "CDynamicProp" ) )
            takedmg = 0;
    }

    const auto ret = _is_breakable.get< bool( __thiscall * )( c_base_entity * ) >( )( ent );

    takedmg = backup_takedmg;

    return ret;
}

bool penetration_system::trace_to_exit( c_game_trace &enter_trace, c_game_trace &exit_trace, vector_3d start_position, vector_3d direction ) {
    static constexpr auto max_distance = 90.f;
    static constexpr auto ray_extension = 4.f;

    float current_distance = 0;
    auto first_contents = 0;

    while ( current_distance <= max_distance ) {
        current_distance += ray_extension;

        auto start = start_position + direction * current_distance;
        const auto point_contents = g_interfaces.engine_trace->get_point_contents_world_only( start, mask_shot_hull | contents_hitbox );

        if ( !first_contents )
            first_contents = point_contents;

        if ( !( point_contents & mask_shot_hull ) || ( ( point_contents & contents_hitbox ) && point_contents != first_contents ) ) {
            const auto end = start - direction * ray_extension;

            ray_t r{ };
            r.init( start, end );

            c_trace_filter_skip_two_entities filter{ };
            filter.skip1 = globals::local_player;
            filter.skip2 = nullptr;

            g_interfaces.engine_trace->trace_ray( r, mask_shot_hull | contents_hitbox, &filter, &exit_trace );

            if ( globals::cvars::sv_clip_penetration_traces_to_players->get_int( ) == 1 )
                UTIL_ClipTraceToPlayers( start, end, mask_shot, nullptr, &exit_trace, -60.f );

            if ( exit_trace.start_solid && exit_trace.surface.flags & surf_hitbox ) {
                r.init( start, start_position );
                filter.skip1 = exit_trace.entity;
                g_interfaces.engine_trace->trace_ray( r, mask_shot_hull | contents_hitbox, &filter, &exit_trace );

                if ( exit_trace.did_hit( ) && !exit_trace.start_solid )
                    return true;
            } else if ( exit_trace.did_hit( ) && !exit_trace.start_solid ) {
                if ( enter_trace.surface.flags & surf_nodraw && is_breakable_entity( static_cast< c_cs_player * >( enter_trace.entity ) ) &&
                     is_breakable_entity( static_cast< c_cs_player * >( exit_trace.entity ) ) )
                    return true;
                else if ( ( !( exit_trace.surface.flags & surf_nodraw ) || enter_trace.surface.flags & surf_nodraw ) && glm::dot( exit_trace.plane.normal, direction ) <= 1.f )
                    return true;
            } else if ( enter_trace.did_hit_non_world_entity( ) && is_breakable_entity( static_cast< c_cs_player * >( enter_trace.entity ) ) ) {
                exit_trace = enter_trace;
                exit_trace.end_pos = start + direction;
                return true;
            }
        }
    }

    return false;
}

bool penetration_system::handle_bullet_penetration( const c_cs_weapon_info *weapon_data, c_game_trace &enter_trace, vector_3d &eye_position, vector_3d direction, int &penetration_count, float &current_damage, float penetration_power ) {
    c_game_trace exit_trace{ };
    auto enemy = reinterpret_cast< c_cs_player * >( enter_trace.entity );
    const auto enter_surface_data = g_interfaces.physics_props->get_surface_data( enter_trace.surface.surface_props );
    const int enter_material = enter_surface_data->game.material;
    const bool is_grate = enter_trace.contents & contents_grate;
    const bool is_no_draw = !!( enter_trace.surface.flags & surf_nodraw );

    if ( weapon_data->penetration <= 0.f || penetration_count <= 0 || ( !trace_to_exit( enter_trace, exit_trace, enter_trace.end_pos, direction ) && ( !( g_interfaces.engine_trace->get_point_contents_world_only( enter_trace.end_pos, mask_shot_hull ) & mask_shot_hull ) ) ) )
        return false;

    const auto exit_surface_data = g_interfaces.physics_props->get_surface_data( exit_trace.surface.surface_props );
    const auto exit_material = exit_surface_data->game.material;

    auto damage_lost = .16f;
    auto penetration_modifier = enter_surface_data->game.penetration_modifier;

    if ( is_grate || is_no_draw || enter_material == char_tex_grate || enter_material == char_tex_glass ) {
        if ( enter_material == char_tex_grate || enter_material == char_tex_glass ) {
            penetration_modifier = 3.f;
            damage_lost = 0.05f;
        } else
            penetration_modifier = 1.f;
    } else if ( enter_material == char_tex_flesh && enemy && ( enemy->team( ) == globals::local_player->team( ) ) && globals::cvars::ff_damage_reduction_bullets->get_float( ) == 0.f ) {
        const auto damage_bullet_penetration = globals::cvars::ff_damage_bullet_penetration->get_float( );

        if ( damage_bullet_penetration == 0.f ) {
            penetration_modifier = 0.0f;
            return false;
        }

        penetration_modifier = damage_bullet_penetration;
    } else
        penetration_modifier = ( penetration_modifier + exit_surface_data->game.penetration_modifier ) / 2.f;

    if ( enter_material == exit_material ) {
        if ( exit_material == char_tex_cardboard || exit_material == char_tex_wood )
            penetration_modifier = 3.f;
        else if ( exit_material == char_tex_plastic )
            penetration_modifier = 2.f;
    }

    const auto dist = glm::length( exit_trace.end_pos - enter_trace.end_pos );
    const auto pen_mod = max( 0.f, ( 1.f / penetration_modifier ) );
    const auto wpn_mod = current_damage * damage_lost + max( 0.f, ( 3.f / penetration_power ) * 1.25f ) * ( pen_mod * 3.f );
    const auto lost_damage = wpn_mod + ( pen_mod * dist * dist ) / 24.f;
    current_damage -= max( 0.f, lost_damage );

    if ( current_damage < 1.f )
        return false;

    eye_position = exit_trace.end_pos;
    --penetration_count;
    current_damage = 0;

    return true;
}

float penetration_system::scale_damage( c_cs_player *player, float damage, float weapon_armor_ratio, int hitgroup, bool is_zeus ) {
    if ( !player->alive( ) )
        return 0.0f;

    auto weapon = g_interfaces.entity_list->get_client_entity_from_handle< c_cs_weapon_base * >( player->weapon_handle( ) );

    if ( !weapon )
        return 0.0f;

    auto weapon_data = weapon->get_weapon_data( );

    if ( !weapon_data )
        return 0.0f;

    float scale_body_damage = ( player->team( ) == 3 ) ? globals::cvars::mp_damage_scale_ct_body->get_float( ) : globals::cvars::mp_damage_scale_t_body->get_float( );
    float head_damage_scale = ( player->team( ) == 3 ) ? globals::cvars::mp_damage_scale_ct_head->get_float( ) : globals::cvars::mp_damage_scale_t_head->get_float( );

    bool armored = false;

    if ( player->armor( ) <= 0 )
        armored = false;
    else {
        switch ( hitgroup ) {
            case hitgroups::hitgroup_generic:
            case hitgroups::hitgroup_chest:
            case hitgroups::hitgroup_stomach:
            case hitgroups::hitgroup_leftarm:
            case hitgroups::hitgroup_rightarm:
                armored = true;
                break;
            case hitgroups::hitgroup_head:
                armored = player->helmet( );
                break;
            default:
                break;
        }
    }

    switch ( hitgroup ) {
        case hitgroup_head:
            if ( player->heavy_armor( ) )
                damage = ( damage * 4.f ) * .5f;
            else
                damage *= 4.f;
            break;
        case hitgroup_stomach:
            damage *= 1.25f;
            break;
        case hitgroup_leftleg:
        case hitgroup_rightleg:
            damage *= .75f;
            break;
        default:
            break;
    }

    if ( armored ) {
        auto modifier = 1.f, armor_bonus_ratio = .5f, armor_ratio = weapon_armor_ratio * .5f;

        if ( player->heavy_armor( ) ) {
            armor_bonus_ratio = .33f;
            armor_ratio = ( weapon_armor_ratio * .5f ) * .5f;
            modifier = .33f;
        }

        auto new_damage = damage * armor_ratio;

        if ( player->heavy_armor( ) )
            new_damage *= .85f;

        if ( ( damage - damage * armor_ratio ) * ( modifier * armor_bonus_ratio ) > player->armor( ) )
            new_damage = damage - player->armor( ) / armor_bonus_ratio;

        damage = new_damage;
    }

    return damage;
}

void penetration_system::UTIL_ClipTraceToPlayers( const vector_3d &start, const vector_3d &end, uint32_t mask, c_trace_filter *filter, c_game_trace *tr, float range ) {
    static auto func = signature::find( _xs( "client.dll" ), _xs( "E8 ? ? ? ? 83 C4 14 8A 56 37" ) ).add( 0x1 ).rel32( ).get< uintptr_t >( );
    if ( !func )
        return;

    __asm {
			mov  ecx, start
			mov	 edx, end
			push range
			push tr
			push filter
			push mask
			call func
			add	 esp, 16
    }
}

void penetration_system::clip_trace_to_player( c_cs_player *player, const vector_3d &start, const vector_3d &end, unsigned int mask, c_trace_filter *filter, c_game_trace &tr, float max_distance ) {
    vector_3d pos, to, dir, on_ray;
    float len, range_along;
    c_game_trace new_trace;

    if ( !player->collideable( ) )
        return;

    pos = player->origin( ) + ( ( player->collideable( )->mins( ) + player->collideable( )->maxs( ) ) * 0.5f );
    to = pos - start;
    dir = start - end;

    float smallest_fraction = tr.fraction;

    if ( filter && filter->should_hit_entity( player, mask ) == false )
        return;

    float range = math::distance_to_ray( pos, start, end );

    ray_t ray;
    c_game_trace player_trace;
    ray.init( start, end );

    if ( range <= 60.f ) {
        g_interfaces.engine_trace->clip_ray_to_entity( ray, mask | contents_hitbox, player, &player_trace );

        if ( player_trace.fraction < smallest_fraction ) {
            tr = player_trace;
            smallest_fraction = player_trace.fraction;
        }
    }
}

void penetration_system::clip_trace_to_players( const vector_3d &start, const vector_3d &end, unsigned int mask, c_trace_filter_hitscan *filter, c_game_trace *tr ) {
    for ( int i = 1; i <= g_interfaces.global_vars->max_clients; i++ ) {
        auto player = g_interfaces.entity_list->get_client_entity< c_cs_player * >( i );

        if ( player )
            clip_trace_to_player( player, start, end, mask, filter, *tr );
    }
}

bool penetration_system::proxy_trace_to_studio_csgo_hitgroups_priority( c_cs_player *ent, uint32_t contents_mask, vector_3d *origin, c_game_trace *tr, ray_t *ray, matrix_3x4 **mat ) {
    const auto studio_model = ent->cstudio_hdr( )->studio_hdr;
    const auto r_ = uintptr_t( ray );
    const auto tr_ = uintptr_t( tr );
    const auto scale_ = ent->model_scale( );
    const auto origin_ = uintptr_t( origin );
    const auto mat_ = uintptr_t( mat );
    const auto set_ = uintptr_t( studio_model->hitbox_set( ent->hitbox_set( ) ) );
    const auto fn_ = g_addresses.trace_to_studio_csgo_hitgroups_priority.get< uintptr_t >( );
    const auto chdr_ = uintptr_t( ent->get_model_ptr( ) );

    auto rval = false;
    __asm
    {
			mov edx, r_
			push tr_
			push scale_
			push origin_
			push contents_mask
			push mat_
			push set_
			push chdr_
			mov eax, [fn_]
			call eax
			add esp, 0x1C
			mov rval, al
    }

    return rval;
}

bool penetration_system::trace_ray( const vector_3d &min, const vector_3d &max, const matrix_3x4 &mat, float r, const vector_3d &src, const vector_3d &dst ) {
    static auto vector_rotate = []( const vector_3d &in1, const matrix_3x4 &in2, vector_3d &out ) {
        out[ 0 ] = in1[ 0 ] * in2[ 0 ][ 0 ] + in1[ 1 ] * in2[ 1 ][ 0 ] + in1[ 2 ] * in2[ 2 ][ 0 ];
        out[ 1 ] = in1[ 0 ] * in2[ 0 ][ 1 ] + in1[ 1 ] * in2[ 1 ][ 1 ] + in1[ 2 ] * in2[ 2 ][ 1 ];
        out[ 2 ] = in1[ 0 ] * in2[ 0 ][ 2 ] + in1[ 1 ] * in2[ 1 ][ 2 ] + in1[ 2 ] * in2[ 2 ][ 2 ];
    };

    static auto vector_transform = []( const vector_3d &in1, const matrix_3x4 &in2, vector_3d &out ) {
        vector_3d in1t;

        in1t[ 0 ] = in1[ 0 ] - in2[ 0 ][ 3 ];
        in1t[ 1 ] = in1[ 1 ] - in2[ 1 ][ 3 ];
        in1t[ 2 ] = in1[ 2 ] - in2[ 2 ][ 3 ];

        vector_rotate( in1t, in2, out );
    };

    static auto trace_aabb = []( const vector_3d &src, const vector_3d &dst, const vector_3d &min, const vector_3d &max ) -> bool {
        auto dir = math::normalize_angle( dst - src );

        if ( dir == vector_3d( ) )
            return false;

        float tmin, tmax, tymin, tymax, tzmin, tzmax;

        if ( dir.x >= 0.0f ) {
            tmin = ( min.x - src.x ) / dir.x;
            tmax = ( max.x - src.x ) / dir.x;
        } else {
            tmin = ( max.x - src.x ) / dir.x;
            tmax = ( min.x - src.x ) / dir.x;
        }

        if ( dir.y >= 0.0f ) {
            tymin = ( min.y - src.y ) / dir.y;
            tymax = ( max.y - src.y ) / dir.y;
        } else {
            tymin = ( max.y - src.y ) / dir.y;
            tymax = ( min.y - src.y ) / dir.y;
        }

        if ( tmin > tymax || tymin > tmax )
            return false;

        if ( tymin > tmin )
            tmin = tymin;

        if ( tymax < tmax )
            tmax = tymax;

        if ( dir.z >= 0.0f ) {
            tzmin = ( min.z - src.z ) / dir.z;
            tzmax = ( max.z - src.z ) / dir.z;
        } else {
            tzmin = ( max.z - src.z ) / dir.z;
            tzmax = ( min.z - src.z ) / dir.z;
        }

        if ( tmin > tzmax || tzmin > tmax )
            return false;

        if ( tmin < 0.0f || tmax < 0.0f )
            return false;

        return true;
    };

    static auto trace_obb = [ & ]( const vector_3d &src, const vector_3d &dst, const vector_3d &min, const vector_3d &max, const matrix_3x4 &mat ) -> bool {
        const auto dir = math::normalize_angle( dst - src );

        vector_3d ray_trans, dir_trans;
        vector_transform( src, mat, ray_trans );
        vector_rotate( dir, mat, dir_trans );

        return trace_aabb( ray_trans, dir_trans, min, max );
    };

    static auto trace_sphere = []( const vector_3d &src, const vector_3d &dst, const vector_3d &sphere, float rad ) -> bool {
        auto delta = math::normalize_angle( dst - src );

        if ( delta == vector_3d( ) )
            return false;

        auto q = sphere - src;

        if ( q == vector_3d( ) )
            return false;

        auto v = glm::dot( q, delta );
        auto d = ( rad * rad ) - ( math::length_sqr( q ) - v * v );

        if ( d < FLT_EPSILON )
            return false;

        return true;
    };

    if ( r == -1.0f ) {
        return trace_obb( src, dst, min, max, mat );
    } else {
        auto delta = math::normalize_angle( max - min );

        const auto hitbox_delta = floorf( glm::length( max - min ) );

        for ( auto i = 0.0f; i <= hitbox_delta; i += 1.0f ) {
            if ( trace_sphere( src, dst, min + delta * i, r ) )
                return true;
        }
    }

    return false;
}