#include "penetration.hpp"

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

c_fire_bullet_data penetration_system::run( const vector_3d src, const vector_3d end, c_cs_player *ent, bool is_zeus, c_cs_weapon_info *info_override ) {
    c_fire_bullet_data info{ };

    if ( !ent )
        return { };

    const auto weapon = g_interfaces.entity_list->get_client_entity_from_handle< c_cs_weapon_base * >( ent->weapon_handle( ) );

    if ( !weapon )
        return { };

    const auto data = info_override == nullptr ? weapon->get_weapon_data( ) : info_override;

    if ( !data )
        return { };

    info.damage = data->damage;
    info.in_damage = g_vars.aimbot_min_damage.value;

    c_trace_filter_hitscan filter{ };

    bool result = simulate_fire_bullet( data, src, end, info, is_zeus, ent );

    if ( result && info.damage < 1.0f )
        return { };

    return info;
}

bool penetration_system::simulate_fire_bullet( const c_cs_weapon_info *data, vector_3d src, vector_3d pos, c_fire_bullet_data &fire_info, bool is_zeus, c_cs_player *ent ) {
    fire_info.penetrate_count = 4;

    float penetration = data->penetration;
    float current_distance = 0.0f, damage_modifier = 0.5f, penetration_power = 35.0f, penetration_modifier = 1.0f;
    float weapon_range = data->range;

    vector_3d dir = math::normalize_angle( pos - src );

    if ( globals::cvars::sv_penetration_type->get_int( ) == 1 )
        penetration_power = penetration;

    int penetration_count = fire_info.penetrate_count;

    c_base_entity *last_hit_entity = nullptr;
    c_trace_filter_skip_two_entities filter_skip2;

    filter_skip2.skip1 = globals::local_player;
    filter_skip2.skip2 = nullptr;

    while ( fire_info.penetrate_count > 0 && fire_info.damage > 0.0f ) {
        float trace_length_remaining = ( weapon_range - current_distance );

        vector_3d end = src + ( dir * trace_length_remaining );

        c_game_trace exit_trace, enter_trace;
        ray_t ray;

        ray.init( src, end );
        filter_skip2.skip1 = globals::local_player;
        filter_skip2.skip2 = last_hit_entity ? last_hit_entity : nullptr;

        g_interfaces.engine_trace->trace_ray( ray, mask_shot_hull | contents_hitbox, &filter_skip2, &enter_trace );

        // get last hit entity for trace.
        last_hit_entity = enter_trace.entity ? enter_trace.entity : nullptr;

        clip_trace_to_player( ent, src, end + ( dir * 40.f ), mask_shot_hull | contents_hitbox, &filter_skip2, &enter_trace );

        if ( enter_trace.fraction == 1.0f )
            break;

        auto surface_data = g_interfaces.physics_props->get_surface_data( enter_trace.surface.surface_props );

        current_distance += enter_trace.fraction * trace_length_remaining;
        fire_info.damage *= std::pow( data->range_modifier, ( current_distance / 500.f ) );

        if ( enter_trace.entity && enter_trace.entity == ent && ( enter_trace.hit_group >= 0 && enter_trace.hit_group <= 7 ) ) {
            fire_info.out_damage = scale_damage( ent, fire_info.damage, data->armor_ratio, enter_trace.hit_group, is_zeus ); /* return our damage if our ray is visible. */

            return fire_info.out_damage >= fire_info.in_damage;
        }

        fire_info.out_damage = data->damage;

        int enter_material = surface_data->game.material;

        penetration_modifier = surface_data->game.penetration_modifier;
        damage_modifier = surface_data->game.damage_modifier;
        penetration = data->penetration;

        if ( ( current_distance > 3000.0f && data->penetration > 0.0f ) || penetration_modifier < 0.1f )
            break;

        bool hit_grate = ( enter_trace.contents & contents_grate ) != 0;

        bool bullet_stopped = handle_bullet_penetration(
                ent,
                penetration,
                enter_material,
                hit_grate,
                enter_trace,
                dir,
                surface_data,
                penetration_modifier,
                damage_modifier,
                penetration_power,
                penetration_count,
                src,
                weapon_range,
                fire_info.trace_length,
                fire_info.damage,
                exit_trace );

        if ( bullet_stopped )
            break;

        fire_info.out_hitgroup = exit_trace.hit_group;
    }

    return true;
}

bool penetration_system::is_breakable_entity( c_cs_player *ent ) {
    static auto _is_breakable = signature::find( XOR( "client.dll" ), XOR( "E8 ? ? ? ? 84 C0 75 A1" ) ).add( 0x1 ).rel32( );
    static auto takedamage_offset = _is_breakable.add( 38 ).deref( ).get< std::uint32_t >( );

    if ( !ent || !ent->index( ) )
        return false;

    auto &takedmg = *reinterpret_cast< std::uint8_t * >( reinterpret_cast< std::uintptr_t >( ent ) + takedamage_offset );

    const auto backup_takedmg = takedmg;
    takedmg = 2;

    const auto ret = _is_breakable.get< bool( __thiscall * )( c_cs_player * ) >( )( ent );

    takedmg = backup_takedmg;

    return ret;
}

bool penetration_system::trace_to_exit( vector_3d start, vector_3d dir, vector_3d &end, c_game_trace &trace_enter, c_game_trace &trace_exit, float step_size, float max_distance ) {
    float dist = 0.f;
    vector_3d last = start;
    int contents, start_contents = 0;

    while ( dist <= max_distance ) {
        dist += step_size;

        end = start + ( dir * dist );

        vector_3d trace_end = end - ( dir * step_size );

        if ( !start_contents )
            start_contents = g_interfaces.engine_trace->get_point_contents( end, mask_shot_hull | contents_hitbox, nullptr );

        contents = g_interfaces.engine_trace->get_point_contents( end, mask_shot_hull | contents_hitbox, nullptr );

        if ( ( contents & 0x600400B ) && ( !( contents & contents_hitbox ) || ( contents == start_contents ) ) )
            continue;

        ray_t ray_world{ };

        ray_world.init( end, trace_end );
        g_interfaces.engine_trace->trace_ray( ray_world, 0x4600400B, nullptr, &trace_exit );

        if ( globals::cvars::sv_clip_penetration_traces_to_players->get_int( ) )
            clip_trace_to_players( end, trace_end, 0x4600400B, nullptr, &trace_exit );

        if ( trace_exit.start_solid && ( trace_exit.surface.flags & surf_hitbox ) ) {
            ray_t ray{ };
            c_trace_filter_world_and_props filter;
            ray.init( end, start );

            g_interfaces.engine_trace->trace_ray( ray, 0x600400B, &filter, &trace_enter );

            if ( trace_exit.did_hit( ) && !trace_exit.start_solid ) {
                end = trace_exit.end_pos;

                return true;
            }
        }

        if ( trace_exit.did_hit( ) && !trace_exit.start_solid ) {
            bool start_is_no_draw = !!( trace_enter.surface.flags & surf_nodraw );
            bool exit_is_no_draw = !!( trace_exit.surface.flags & surf_nodraw );

            if ( exit_is_no_draw && is_breakable_entity( static_cast< c_cs_player * >( trace_enter.entity ) ) && is_breakable_entity( static_cast< c_cs_player * >( trace_enter.entity ) ) ) {
                end = trace_exit.end_pos;
                return true;
            }

            if ( exit_is_no_draw == false || ( start_is_no_draw && exit_is_no_draw ) ) {
                float dot_val = glm::dot( dir, trace_exit.plane.normal );

                if ( dot_val <= 1.0f ) {
                    end = end - ( dir * ( step_size * trace_exit.fraction ) );

                    return true;
                }
            }
        }

        if ( trace_enter.did_hit_non_world_entity( ) && is_breakable_entity( static_cast< c_cs_player * >( trace_enter.entity ) ) ) {
            trace_exit = trace_enter;
            trace_exit.end_pos = start + dir;

            return true;
        }
    }

    return false;
}

bool penetration_system::handle_bullet_penetration( c_cs_player *ent, float &penetration, int &enter_material, bool &hit_grate, c_game_trace &tr, vector_3d &direction, surfacedata_t *surface_data, float penetration_modifier, float damage_modifier, float penetration_power, int &penetration_count, vector_3d &src, float distance, float current_distance, float &current_damage, c_game_trace &exit_trace ) {
    bool is_no_draw = !!( tr.surface.flags & surf_nodraw );

    if ( penetration_count == 0 && !hit_grate && !is_no_draw && enter_material != char_tex_glass && enter_material != char_tex_grate )
        return false;

    if ( penetration <= 0 || penetration_count <= 0 )
        return false;

    vector_3d penetration_end;

    if ( !trace_to_exit( tr.end_pos, direction, penetration_end, tr, exit_trace, 4.0f, 90.0f ) ) {
        if ( ( g_interfaces.engine_trace->get_point_contents( tr.end_pos, mask_shot_hull ) & mask_shot_hull ) == 0 ) {
            return false;
        }
    }

    surfacedata_t *exit_surface_data = g_interfaces.physics_props->get_surface_data( exit_trace.surface.surface_props );

    int exit_material = exit_surface_data->game.material;

    float damage_lost_percentage = 0.16f;

    if ( hit_grate || is_no_draw || enter_material == char_tex_glass || enter_material == char_tex_grate ) {
        if ( enter_material == char_tex_glass || enter_material == char_tex_grate ) {
            penetration_modifier = 3.0f;
            damage_lost_percentage = 0.05f;
        } else
            penetration_modifier = 1.0f;

        damage_modifier = 0.99f;
    } else if ( enter_material == char_tex_flesh && globals::cvars::ff_damage_reduction_bullets->get_float( ) == 0 && ( ent && tr.entity && static_cast< c_cs_player * >( tr.entity )->is_player( ) && tr.entity->team( ) == ent->team( ) ) ) {
        if ( globals::cvars::ff_damage_bullet_penetration->get_float( ) == 0 ) {
            penetration_modifier = 0.f;
            return true;
        }

        penetration_modifier = globals::cvars::ff_damage_bullet_penetration->get_float( );
    } else {
        float exit_penetration_modifier = exit_surface_data->game.penetration_modifier;
        float exit_damage_modifier = exit_surface_data->game.damage_modifier;

        penetration_modifier = ( penetration_modifier + exit_penetration_modifier ) / 2;
    }

    if ( enter_material == exit_material ) {
        if ( exit_material == char_tex_wood || exit_material == char_tex_cardboard ) {
            penetration_modifier = 3.f;
        } else if ( exit_material == char_tex_plastic ) {
            penetration_modifier = 2.f;
        }
    }

    float trace_distance = glm::length( exit_trace.end_pos - tr.end_pos );
    float penetration_mod = std::fmaxf( 0.f, ( 1.f / penetration_modifier ) );
    float percent_damage_chunk = current_damage * damage_lost_percentage;
    float pen_wep_mod = percent_damage_chunk + std::fmaxf( 0.f, ( 3.f / penetration_power ) * 1.25f ) * ( penetration_mod * 3.0f );
    float lost_damage_object = ( ( penetration_mod * ( trace_distance * trace_distance ) ) / 24.f );
    float total_lost_damage = pen_wep_mod + lost_damage_object;

    current_damage -= std::fmaxf( 0.f, total_lost_damage );

    if ( current_damage < 1.f )
        return false;

    src = exit_trace.end_pos;
    penetration_count--;

    return true;
}

float penetration_system::scale_damage( c_cs_player *player, float damage, float armor_ratio, int hitgroup, bool is_zeus ) {
    auto weapon = g_interfaces.entity_list->get_client_entity_from_handle< c_cs_weapon_base * >( player->weapon_handle( ) );

    if ( !weapon )
        return 0.0f;

    auto weapon_data = weapon->get_weapon_data( );

    if ( !weapon_data )
        return 0.0f;

    float scale_body_damage = ( player->team( ) == 3 ) ? globals::cvars::mp_damage_scale_ct_body->get_float( ) : globals::cvars::mp_damage_scale_t_body->get_float( );
    float head_damage_scale = ( player->team( ) == 3 ) ? globals::cvars::mp_damage_scale_ct_head->get_float( ) : globals::cvars::mp_damage_scale_t_head->get_float( );

    static auto is_armored = [ ]( c_cs_player *player, int hitgroup ) -> bool {
        if ( player->armor( ) <= 0 )
            return false;

        switch ( hitgroup ) {
            case hitgroups::hitgroup_generic:
            case hitgroups::hitgroup_chest:
            case hitgroups::hitgroup_stomach:
            case hitgroups::hitgroup_leftarm:
            case hitgroups::hitgroup_rightarm:
                return true;
                break;
            case hitgroups::hitgroup_head:
                if ( player->helmet( ) )
                    return true;
                break;
            default:
                break;
        }

        return false;
    };

    const int armor = player->armor( );

    if ( player->heavy_armor( ) )
        head_damage_scale *= 0.5f;

    if ( !is_zeus )
        switch ( hitgroup ) {
            case hitgroups::hitgroup_head:
                damage *= 4.0f * head_damage_scale;
                break;
            case hitgroups::hitgroup_chest:
                damage *= 1.0f * scale_body_damage;
                break;
            case hitgroups::hitgroup_stomach:
                damage *= 1.25f * scale_body_damage;
                break;
            case hitgroups::hitgroup_leftarm:
            case hitgroups::hitgroup_rightarm:
                damage *= 1.0f * scale_body_damage;
                break;
            case hitgroups::hitgroup_leftleg:
            case hitgroups::hitgroup_rightleg:
                damage *= 0.75f * scale_body_damage;
                break;
            default:
                break;
        }

    if ( is_armored( player, hitgroup ) ) {
        float armor_bonus = 0.5f, armor_ratio = weapon_data->armor_ratio / 2.0f, heavy_armor_bonus = 1.0f;
        bool has_heavy = player->heavy_armor( );

        if ( has_heavy ) {
            armor_ratio *= 0.5f;
            armor_bonus = 0.33f;
            heavy_armor_bonus = 0.33f;
        }

        float damage_to_health = damage * armor_ratio;
        float damage_to_armor = ( damage - damage_to_health ) * ( armor_bonus * heavy_armor_bonus );

        if ( damage_to_armor > static_cast< float >( armor ) )
            damage_to_health = damage - static_cast< float >( armor ) / armor_bonus;

        return damage_to_health;
    }

    return 0.0f;
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