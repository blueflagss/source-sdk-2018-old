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

c_fire_bullet_data penetration_system::run( const vector_3d src, const vector_3d end, c_cs_player *ent, const std::array< matrix_3x4, 128 > &bones, bool is_zeus, c_cs_weapon_info *info_override ) {
    c_fire_bullet_data info{ };

    if ( !ent )
        return { };

    if ( !globals::local_weapon )
        return { };

    const auto data = info_override == nullptr ? globals::local_weapon->get_weapon_data( ) : info_override;

    if ( !data )
        return { };

    info.damage = data->damage;

    c_trace_filter_hitscan filter{ };

    bool result = simulate_fire_bullet( data, src, end, info, bones, is_zeus, ent );

    if ( !result || info.damage < 1.0f )
        return { };

    return info;
}

bool penetration_system::simulate_fire_bullet( const c_cs_weapon_info *data, vector_3d src, vector_3d pos, c_fire_bullet_data &fire_info, const std::array< matrix_3x4, 128 > &bones, bool is_zeus, c_cs_player *ent ) {
    vector_3d direction = math::normalize_angle( pos - src );

    fire_info.penetrate_count = 4;
    auto length = 0.f;
    c_game_trace enter_trace{ }, final_trace{ };

    c_trace_filter_hitscan filter;
    filter.player = globals::local_player;

    matrix_3x4 *temp_mat[ 128 ];

    ray_t r{ };
    r.init( src, src + direction * ( data->range + ray_extension ) );

    c_game_trace tr{ };
    tr.start_pos = r.start + r.start_offset;
    tr.end_pos = tr.start_pos + r.delta;

    for ( int i = 0; i < bones.size( ); i++ )
        temp_mat[ i ] = const_cast< matrix_3x4 * >( &bones[ i ] );

    const auto ret = proxy_trace_to_studio_csgo_hitgroups_priority( ent, mask_shot_hull | contents_hitbox, &ent->origin( ), &tr, &r, temp_mat );

    fire_info.did_hit = ret;

    final_trace = tr;

    if ( !fire_info.did_hit )
        return false;

    fire_info.did_hit = false;

    while ( fire_info.penetrate_count > 0 && fire_info.damage > 0.0f ) {
        const auto length_remaining = data->range - length;
        auto end = src + direction * length_remaining;

        if ( enter_trace.entity )
            filter.player = enter_trace.entity;

        ray_t r{ };
        r.init( src, end );
        g_interfaces.engine_trace->trace_ray( r, mask_shot, &filter, &enter_trace );

        if ( !fire_info.did_hit ) {
            const auto dist = glm::length( src - ent->get_abs_origin( ) );
            const auto behind = glm::length( src - enter_trace.end_pos ) > dist;

            if ( behind || enter_trace.fraction == 1.f ) {
                ray_t r2{ };
                r2.init( src, src + direction * ( data->range + ray_extension ) );
                final_trace.fraction = glm::length( final_trace.end_pos - r2.start ) / r2.delta.length( );
                enter_trace = final_trace;

                auto smallest_fraction = glm::length( src - enter_trace.end_pos ) / length_remaining;
                const auto final_length = length + smallest_fraction * ( length_remaining + ray_extension );

                if ( final_length >= data->range )
                    break;

                const auto final_damage = fire_info.damage * std::powf( data->range_modifier, final_length * 0.002f );

                fire_info.did_hit = true;
                fire_info.bullet_end = enter_trace.end_pos;
                fire_info.impacts[ fire_info.impact_count++ ] = enter_trace.end_pos;
                fire_info.out_damage = scale_damage( ent, final_damage, data->armor_ratio, enter_trace.hit_group, is_zeus );
                fire_info.out_hitgroup = enter_trace.hit_group;

                break;
            }
        }

        length += enter_trace.fraction * length_remaining;
        fire_info.damage *= std::powf( data->range_modifier, length * 0.002f );

        if ( length > glm::length( src - pos ) )
            return false;

        if ( enter_trace.fraction == 1.f )
            return false;

        fire_info.impacts[ fire_info.impact_count++ ] = enter_trace.end_pos;

        const auto enter_surf = g_interfaces.physics_props->get_surface_data( enter_trace.surface.surface_props );
        if ( ( length > 3000.f && data->penetration > 0.f ) || ( !enter_surf || enter_surf->game.penetration_modifier < 0.1f ) )
            fire_info.penetrate_count = 0;

        if ( !handle_bullet_penetration( data, enter_trace, src, direction, fire_info.penetrate_count, fire_info.damage, data->penetration ) ) {
            fire_info.damage = 0.f;
            return false;
        }
    }

    if ( !fire_info.did_hit )
        fire_info.out_damage = fire_info.damage;
    else {
        return true;
    }
}

bool penetration_system::is_breakable_entity( c_cs_player *ent ) {
    static auto _is_breakable = signature::find( XOR( "client.dll" ), XOR( "E8 ? ? ? ? 84 C0 75 A1" ) ).add( 0x1 ).rel32( );
    static auto takedamage_offset = _is_breakable.add( 38 ).deref( ).get< std::uint32_t >( );

    if ( !ent || !ent->index( ) )
        return false;

    auto &takedmg = *reinterpret_cast< std::uint8_t * >( reinterpret_cast< std::uintptr_t >( ent ) + takedamage_offset );

    const auto backup_takedmg = takedmg;

    auto cc = ent->get_client_class( );

    if ( cc ) {
        auto name = cc->network_name;

        if ( name[ 1 ] != 'F' || name[ 4 ] != 'c' || name[ 5 ] != 'B' || name[ 9 ] != 'h' )
            takedmg = 2;
    }

    const auto ret = _is_breakable.get< bool( __thiscall * )( c_cs_player * ) >( )( ent );

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

    if ( weapon_data->penetration <= 0.f || penetration_count <= 0 || ( !trace_to_exit( enter_trace, exit_trace, enter_trace.end_pos, direction ) && ( !( g_interfaces.engine_trace->get_point_contents( enter_trace.end_pos, mask_shot_hull ) & mask_shot_hull ) ) ) )
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
        if ( damage_bullet_penetration == 0.f )
            return false;

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

void penetration_system::clip_trace_to_player( c_cs_player *player, const vector_3d &start, const vector_3d &end, unsigned int mask, c_trace_filter *filter, c_game_trace *tr ) {
    if ( !player || !player->alive( ) || player->dormant( ) )
        return;

    float smallest_fraction = tr->fraction;
    const float max_range = 60.0f;

    ray_t ray;
    c_game_trace player_trace;

    ray.init( start, end );

    vector_3d world_space_center = player->origin( ) + ( ( player->mins( ) + player->maxs( ) ) * 0.5f );

    if ( filter && filter->should_hit_entity( player, mask ) == false )
        return;

    float range = math::distance_to_ray( world_space_center, start, end );

    if ( range < 0.0f || range > max_range )
        return;

    g_interfaces.engine_trace->clip_ray_to_entity( ray, mask | contents_hitbox, player, &player_trace );

    if ( player_trace.fraction < smallest_fraction ) {
        *tr = player_trace;
        smallest_fraction = player_trace.fraction;
    }
}

void penetration_system::clip_trace_to_players( const vector_3d &start, const vector_3d &end, unsigned int mask, c_trace_filter_hitscan *filter, c_game_trace *tr ) {
    for ( int i = 1; i <= g_interfaces.global_vars->max_clients; i++ ) {
        auto player = g_interfaces.entity_list->get_client_entity< c_cs_player * >( i );

        if ( player )
            clip_trace_to_player( player, start, end, mask, filter, tr );
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
    const auto chdr_ = uintptr_t( ent->cstudio_hdr( ) );

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