#include "visuals.hpp"
#include <features/features.hpp>

void visuals::render( ) {
    if ( !g_interfaces.engine_client->is_in_game( ) || !g_interfaces.engine_client->is_connected( ) )
        return;

    if ( !globals::local_player )
        return;

    for ( int n = 1; n <= ( g_interfaces.entity_list->get_highest_entity_index( ) + 1 ); ++n ) {
        opacity_array.resize( g_interfaces.entity_list->get_highest_entity_index( ) + 1 );

        auto entity = g_interfaces.entity_list->get_client_entity< c_base_entity * >( n );

        if ( !entity )
            continue;

        auto network_name = HASH( entity->get_client_class( )->network_name );

        switch ( network_name ) {
            case HASH_CT( "CCSPlayer" ): {
                const auto player = entity->get< c_cs_player * >( );

                if ( !player || player == globals::local_player || player->team( ) == globals::local_player->team( ) )
                    break;

                update_configuration( player );

                           if ( !player->alive( ) )
                    break;

                if ( !g_vars.visual_players_toggled.value )
                    break;

                render_player( player );
            }
            default: {
                //const auto weapon = entity->get< c_cs_weapon_base * >( );

                //if ( !weapon->is_base_combat_weapon( ) )
                //    break;

                //const auto weapon_owner = g_interfaces.entity_list->get_client_entity_from_handle< c_cs_player * >( weapon->owner( ) );

                //if ( weapon_owner )
                //    break;

                //render_weapon( weapon );
            } break;
        }
    }

    render_scope_lines( );
    hitmarker( );
    indicators( );
    penetration_crosshair( );
}

void visuals::render_scope_lines( ) {
    if ( !g_vars.visuals_other_remove_scope_overlay.value )
        return;

    if ( !globals::local_player->scoped( ) )
        return;

    vector_4d area = {
            globals::ui::screen_size.x,
            globals::ui::screen_size.y,
            globals::ui::screen_size.x / 2.0f,
            globals::ui::screen_size.y / 2.0f
    };

    render::filled_rect( area.z, 0, 1.0f, area.y, color::black( ) );
    render::filled_rect( 0, area.w, area.x, 1.0f, color::black( ) );
}

void visuals::penetration_crosshair( ) {
    if ( !g_vars.visuals_other_penetration_crosshair.value )
        return;

    if ( !g_interfaces.engine_client->is_in_game( ) || !g_interfaces.engine_client->is_connected( ) )
        return;

    if ( !globals::local_player || !globals::local_player->alive( ) )
        return;

    const auto position = vector_2d( globals::ui::screen_size.x / 2.0f, globals::ui::screen_size.y / 2.0f );
    const auto pen_dot_color = ( pen_data.damage > 0.f ) ? color( 0, 255, 0, 200 ) : color( 255, 255, 255, 200 );

    render::rect( position.x, position.y - 1.0f, 1, 3, pen_dot_color );
    render::rect( position.x - 1.0f, position.y, 3, 1, pen_dot_color );
}

void visuals::on_create_move( ) {
    if ( !g_interfaces.engine_client->is_in_game( ) || !g_interfaces.engine_client->is_connected( ) )
        return;

    if ( !globals::local_player || !globals::local_player->alive( ) )
        return;

    if ( g_vars.visuals_other_penetration_crosshair.value ) {
        const auto forward = math::angle_vectors( globals::view_angles );
        const auto start = globals::local_player->get_shoot_position( );
        const auto end = start + ( forward * 8196.0f );

        /* store penetration crosshair data. */
        auto results = g_penetration.run( start, end, globals::local_player, g_animations.animated_bones[ globals::local_player->index( ) ], false, nullptr );

        pen_data.damage = results.out_damage;
    }
}

void visuals::indicators( ) {
    std::deque< std::tuple< const char *, color, float > > indicators{ };

    if ( !g_interfaces.engine_client->is_in_game( ) || !globals::local_player->alive( ) )
        return;

    const auto time = std::clamp< float >( g_animations.lower_body_realign_timer - g_interfaces.global_vars->curtime, 0.0f, 1.0f );

    indicators.emplace_back( std::tuple< const char *, color, float >{ _xs( "LBY" ), color( 200, 200, 200 ), time } );

    if ( indicators.empty( ) )
        return;

    for ( int i = { 0 }; i < indicators.size( ); ++i ) {
        auto &indicator = indicators[ i ];

        const auto text_dimensions = render::get_text_size( fonts::visuals_indicators, std::get< 0 >( indicator ) );
        const auto position = vector_2d( 8.0f, ( globals::ui::screen_size.y / 2.0f ) + ( i * ( text_dimensions.y + 4 ) ) );
        const auto indicator_name = std::get< 0 >( indicator );

        switch ( HASH( indicator_name ) ) {
            case HASH_CT( "LBY" ): {
                render::string( fonts::visuals_indicators, position, color( 0, 0, 0, 190 ), indicator_name );

                render::scissor_rect( position, vector_2d( text_dimensions.x * ( 1.f - time ), text_dimensions.y ), [ & ] {
                    render::string( fonts::visuals_indicators, position, color( 137, 195, 49 ).lerp( color( 186, 1, 1 ), time ), indicator_name, true );
                } );
            } break;
            default: {
                render::string( fonts::visuals_indicators, position, std::get< 1 >( indicator ), indicator_name );
            } break;
        }
    }
}

void visuals::hitmarker( ) {
    if ( !g_vars.misc_hitmarker.value )
        return;

    if ( hitmarker_fraction <= 0.0f )
        return;

    constexpr int line_size = 13;

    auto screen_center = vector_2d(
            globals::ui::screen_size.x / 2.0f,
            globals::ui::screen_size.y / 2.0f 
    );

    render::line( screen_center.x - line_size, screen_center.y - line_size, screen_center.x - ( line_size / 2 ), screen_center.y - ( line_size / 2 ), color{ 200, 200, 200, 255 * hitmarker_fraction }, 2.5f );
    render::line( screen_center.x - line_size, screen_center.y + line_size, screen_center.x - ( line_size / 2 ), screen_center.y + ( line_size / 2 ), color{ 200, 200, 200, 255 * hitmarker_fraction }, 2.5f );
    render::line( screen_center.x + line_size, screen_center.y + line_size, screen_center.x + ( line_size / 2 ), screen_center.y + ( line_size / 2 ), color{ 200, 200, 200, 255 * hitmarker_fraction }, 2.5f );
    render::line( screen_center.x + line_size, screen_center.y - line_size, screen_center.x + ( line_size / 2 ), screen_center.y - ( line_size / 2 ), color{ 200, 200, 200, 255 * hitmarker_fraction }, 2.5f );

    hitmarker_fraction -= std::clamp< float >( ( 1.0f / 0.6f ) * ImGui::GetIO( ).DeltaTime, 0.0f, 1.0f );
}

void visuals::update_configuration( c_base_entity *entity ) {
    const auto dormant = entity->dormant( );
    const auto dormant_color = color( 240, 240, 240, 200 * opacity_array[ entity->index( ) ] );

    opacity_array[ entity->index( ) ] = 1.0f;

    esp_config[ entity->index( ) ].bottom_bar_offset = 0.0f;
    esp_config[ entity->index( ) ].weapon_offset = 0.0f;
    esp_config[ entity->index( ) ].dormant = dormant;
    esp_config[ entity->index( ) ].box_color = dormant ? dormant_color : g_vars.visuals_box_color.value;
    esp_config[ entity->index( ) ].name_color = dormant ? dormant_color : g_vars.visuals_name_color.value;
    esp_config[ entity->index( ) ].weapon_name_color = dormant ? dormant_color : color( 255, 255, 255 );
    esp_config[ entity->index( ) ].offscreen_color = dormant ? dormant_color : g_vars.visuals_other_oof_arrows_color.value;
}

void visuals::render_weapon( c_cs_weapon_base *weapon ) {
    entity_box box;

    if ( weapon->dormant( ) )
        return;

    if ( !weapon->get_screen_bounding_box( box ) )
        return;

    int bar_offset = 0;

    const auto max_distance = 58.0f;
    const auto origin = weapon->origin( );
    const auto distance_to_player = glm::length( origin - globals::local_player->origin( ) ) * 0.01905f;
    const auto max_distance_clamped = ( max_distance - std::clamp< float >( distance_to_player, 0, max_distance ) ) / max_distance;

    if ( max_distance_clamped < 0.02f )
        return;

    render::rect( box.x + 1, box.y + 1, box.w - 2, box.h - 2, color{ 46, 46, 46, 160 * max_distance_clamped } );
    render::rect( box.x - 1, box.y - 1, box.w + 2, box.h + 2, color{ 46, 46, 46, 160 * max_distance_clamped } );
    render::rect( box.x, box.y, box.w, box.h, color{ g_vars.visuals_box_color.value, 200 * max_distance_clamped } );

    auto weapon_text = utils::convert_utf8( weapon->get_name( ) );
    auto text_dimensions = render::get_text_size( fonts::visuals_04b03, weapon_text );

    std::transform( weapon_text.begin( ), weapon_text.end( ), weapon_text.begin( ), ::toupper );

    //const auto progress = static_cast< float >( weapon->clip_1( ) / weapon->primary_reserve_ammo_count( ) );
    //const auto width = ( box.w * progress );

    //if ( progress <= 0.0f ) {
    //    bar_offset = 0;
    //    return;
    //}

    //render::filled_rect( box.x + 1.0f, box.y + box.h + 2.0f + bar_offset, box.w, 2.5f, color{ 46, 46, 46, 110 * max_distance_clamped } );
    //render::filled_rect( box.x + 1.0f, box.y + box.h + 2.0f + bar_offset, width - 2.0f, 4.0f, color{ 255, 255, 255, 200 * max_distance_clamped } );
    //render::rect( box.x + 1.0f, box.y + box.h + 2.0f + bar_offset, box.w - 2.0f, 4.0f, color{ 46, 46, 46, 255 * max_distance_clamped } );

    //if ( progress < 1.0f )
    //    render::string( fonts::visuals_04b03, box.x + 1.0f + box.w, box.y + box.h + 2.0f + 7.0f + bar_offset, color{ color::white( ), 255 * max_distance_clamped }, fmt::format( _xs( "{}" ), weapon->clip_1( ) ), false, true );

    bar_offset += 5.5f;
    render::string( fonts::visuals_04b03, box.x + ( box.w / 2 ) - text_dimensions.x / 2, box.y + bar_offset + box.h + 1.5f, color( 255, 255, 255, 255 * max_distance_clamped ), weapon_text, false, true );
}

void visuals::render_player( c_cs_player *player ) {
    if ( opacity_array[ player->index( ) ] <= 0.02f )
        return;

    render_offscreen( player );

    const auto player_log = &g_animations.player_log[ player->index( ) ];

    if ( !player_log )
        return;

    entity_box box;

    if ( !player->get_screen_bounding_box( box ) )
        return;

    player_info_t player_info;
    g_interfaces.engine_client->get_player_info( player->index( ), &player_info );

    if ( g_vars.visuals_player_name.value ) {
        auto name = std::string( player_info.name );
        auto name_dimensions = render::get_text_size( fonts::visuals_segoe_ui, name );

        render::string( fonts::visuals_segoe_ui, box.x + ( box.w / 2 ) - name_dimensions.x / 2, box.y - name_dimensions.y - 1.5f, color{ esp_config[ player->index( ) ].name_color, 220 * opacity_array[ player->index( ) ] }, name, true );
    }

    if ( g_vars.visuals_player_box.value ) {
        render::rect( box.x + 1, box.y + 1, box.w - 2, box.h - 2, color{ 46, 46, 46, 160 * opacity_array[ player->index( ) ] } );
        render::rect( box.x - 1, box.y - 1, box.w + 2, box.h + 2, color{ 46, 46, 46, 160 * opacity_array[ player->index( ) ] } );
        render::rect( box.x, box.y, box.w, box.h, color{ esp_config[ player->index( ) ].box_color, 200 * opacity_array[ player->index( ) ] } );
    }

    if ( !player->dormant( ) ) {
        std::array< matrix_3x4, 128 > history_bones;

        if ( g_vars.visuals_player_skeleton.value )
            render_skeleton( player, nullptr, g_vars.visuals_skeleton_color.value );

        if ( g_vars.visuals_player_skeleton_history.value && !g_animations.get_lagcomp_bones( player, history_bones ) )
            render_skeleton( player, history_bones.data( ), g_vars.visuals_skeleton_history_color.value );
    }

    const auto dormant = player->dormant( );

    if ( g_vars.visuals_player_health.value ) {
        auto health = player->health( );

        if ( health > 100 )
            health = 100;

        const auto health_bar_delta = static_cast< int >( health * box.h / 100 );
        const auto health_text = fmt::format( _xs( "{}" ), health );
        const auto health_text_dimensions = render::get_text_size( fonts::visuals_04b03, health_text );
        const auto health_bar_color = g_vars.visuals_player_health_override.value ? color{ g_vars.visuals_health_override_color.value, 255 * opacity_array[ player->index( ) ] } : color{ 166, 0, 0, 255 * opacity_array[ player->index( ) ] }.lerp( color{ 157, 255, 0, 255 * opacity_array[ player->index( ) ] }, std::clamp< float >( static_cast< float >( player->health( ) ) / 100.f, 0.0f, 1.0f ) );

        render::filled_rect( box.x - 6.0f, box.y, 2.5f, box.h, color{ 46, 46, 46, 110 * opacity_array[ player->index( ) ] } );

        if ( dormant ) {
            render::filled_rect( box.x - 6.0f, box.y - 1, 2.5f, box.h, color{ 189, 189, 189, 200 * opacity_array[ player->index( ) ] } );
        }

        else {
            render::filled_rect( box.x - 6.0f, box.y - 1 + box.h - health_bar_delta, 2.5f, health_bar_delta, color{ health_bar_color, 200 * opacity_array[ player->index( ) ] } );
        }

        render::rect( box.x - 7.0f, box.y - 1, 4.f, box.h + 2, color{ 46, 46, 46, 190 * opacity_array[ player->index( ) ] } );

        if ( !dormant && health > 0 && health < 100 )
            render::string( fonts::visuals_04b03, box.x - 15.0f + ( health_text_dimensions.x / 2 ), box.y - 4 + box.h - health_bar_delta, color{ color::white( ), 255 * opacity_array[ player->index( ) ] }, health_text, false, true );
    }

    if ( g_vars.visuals_player_lby_timer.value && !player_log->anim_records.empty( ) ) {
        const auto current = &player_log->anim_records.front( );

        if ( current && ( ( math::length_2d( current->velocity ) < 0.2f ) && current->on_ground ) || current->fake_walk ) {
            const auto cycle = std::clamp< float >( player_log->body_update_time - current->sim_time, 0.0f, 1.0f );

            render_bar( player, box, 0.0f, cycle, g_vars.visuals_lby_timer_color.value );
        }
    }

    auto weapon = g_interfaces.entity_list->get_client_entity_from_handle< c_cs_weapon_base * >( player->weapon_handle( ) );

    if ( weapon ) {
        if ( g_vars.visuals_player_weapon_text.value ) {
            auto weapon_text = utils::convert_utf8( weapon->get_name( ) );
            auto text_dimensions = render::get_text_size( fonts::visuals_04b03, weapon_text );

            std::transform( weapon_text.begin( ), weapon_text.end( ), weapon_text.begin( ), ::toupper );

            render::string( fonts::visuals_04b03, box.x + ( box.w / 2 ) - text_dimensions.x / 2, box.y + esp_config[ player->index( ) ].bottom_bar_offset + esp_config[ player->index( ) ].weapon_offset + box.h + 1.5f, color( esp_config[ player->index( ) ].weapon_name_color, 255 ), weapon_text, false, true );

            esp_config[ player->index( ) ].bottom_bar_offset += text_dimensions.y;
        }

        if ( g_vars.visuals_player_weapon_icon.value ) {
            auto icon = csgo_hud_icons[ weapon->item_definition_index( ) ];
            auto text_dimensions = render::get_text_size( fonts::csgo_icons, icon );

            render::string( fonts::csgo_icons, box.x + ( box.w / 2 ) - text_dimensions.x / 2, box.y + esp_config[ player->index( ) ].bottom_bar_offset + esp_config[ player->index( ) ].weapon_offset + box.h, color( 255, 255, 255, 255 ), icon, true, false );
        }
    }

    std::deque< std::pair< std::string, color > > flags;

    if ( !player->dormant( ) ) {
        const auto distance_to_player = glm::length( player->origin( ) - globals::local_player->origin( ) ) * 0.01905f;
        const auto distance_to_player_text = fmt::format( _xs( "{}M" ), static_cast< int >( std::roundf( distance_to_player ) ) );
        const auto text_dimensions = render::get_text_size( fonts::visuals_04b03, distance_to_player_text );

        if ( g_vars.visuals_player_distance.value )
            flags.emplace_back( std::pair< std::string, color >{ distance_to_player_text, { 255, 255, 255 } } );

        if ( g_vars.visuals_player_flags_bot.value && player_info.fake_player )
            flags.emplace_back( std::pair< std::string, color >{ _xs( "BOT" ), { 255, 255, 255 } } );

        if ( g_vars.visuals_player_flags_scoped.value && player->scoped( ) )
            flags.emplace_back( std::pair< std::string, color >{ _xs( "ZOOM" ), { 255, 255, 255 } } );

        auto get_armor_type = []( c_cs_player *player ) -> std::string {
            if ( player->armor( ) > 0 && player->helmet( ) )
                return _xs( "HK" );

            else if ( player->armor( ) )
                return _xs( "K" );

            else if ( player->helmet( ) )
                return _xs( "H" );

            return _xs( "" );
        };

        if ( g_vars.visuals_player_flags_money.value && player->account( ) > 0 )
            flags.emplace_back( std::pair< std::string, color >{ fmt::format( _xs( "${}" ), player->account( ) ), { 255, 255, 255 } } );

        if ( g_vars.visuals_player_flags_lag_amount.value && ( globals::local_player->alive( ) && !player_log->anim_records.empty( ) && !player_info.fake_player ) ) {
            auto current_record = &player_log->anim_records.front( );

            if ( current_record ) {
                if ( g_animations.player_log[ player->index( ) ].anim_records.size( ) > 1 ) {
                    auto &log = g_animations.player_log[ player->index( ) ].anim_records.front( );

                    flags.emplace_back( std::pair< std::string, color >{ fmt::format( _xs( "{}" ), log.choked ), { 255, 255, 255 } } );

                    if ( log.is_exploit )
                        flags.emplace_back( std::pair< std::string, color >{ _xs( "EXPLOIT" ), { 255, 255, 255 } } );
                }
            }
        }

        if ( g_vars.visuals_player_flags_armor.value )
            flags.emplace_back( std::pair< std::string, color >{ get_armor_type( player ), { 255, 255, 255 } } );

        for ( size_t i = { 0ul }; i < flags.size( ); ++i ) {
            auto &flag_object = flags.at( i );
            auto offset = ( i * ( render::get_text_size( fonts::visuals_04b03, flags[ i ].first ).y + 1.0f ) );

            render::string( fonts::visuals_04b03, box.x + box.w + 2, box.y + offset - 1, color{ color::white( ), 255 * opacity_array[ player->index( ) ] }, flag_object.first, false, true );
        }
    }
}

void visuals::world_modulation( ) {
    static auto load_named_sky = signature::find( "engine.dll", _xs( "55 8B EC 81 ? ? ? ? ? 56 57 8B F9 C7" ) ).get< bool( __thiscall * )( const char * ) >( );

    static auto last_world_color = g_vars.visuals_other_modulate_world_color.value;
    static auto last_alive = false;
    static auto last_skybox = g_vars.visuals_other_skybox_selection.value;
    static auto last_modulate_world_enable = g_vars.visuals_other_modulate_world.value;
    static auto last_fullbright_enable = g_vars.visuals_other_fullbright.value;

    if ( g_vars.visuals_other_fullbright.value != last_fullbright_enable ) {
        globals::cvars::mat_fullbright->set_int( g_vars.visuals_other_fullbright.value );

        last_fullbright_enable = g_vars.visuals_other_fullbright.value;
    }

    if ( last_alive != globals::local_player->alive( ) || last_world_color.r != g_vars.visuals_other_modulate_world_color.value.r || last_world_color.g != g_vars.visuals_other_modulate_world_color.value.g || last_world_color.b != g_vars.visuals_other_modulate_world_color.value.b || last_skybox != g_vars.visuals_other_skybox_selection.value || last_modulate_world_enable != g_vars.visuals_other_modulate_world.value ) {
        load_named_sky( globals::sky_names[ g_vars.visuals_other_skybox_selection.value ] );

        if ( g_vars.visuals_other_modulate_world.value || last_world_color != g_vars.visuals_other_modulate_world.value ) {
            for ( uint16_t h = g_interfaces.material_system->first_material( ); h != g_interfaces.material_system->invalid_material( ); h = g_interfaces.material_system->next_material( h ) ) {
                auto mat = g_interfaces.material_system->get_material( h );

                if ( !mat )
                    continue;

                if ( HASH( mat->get_texture_group_name( ) ) == HASH_CT( "World textures" ) && g_vars.visuals_other_modulate_world_color.value != color( 255, 255, 255 ) )
                    mat->color_modulate( g_vars.visuals_other_modulate_world_color.value.r / 255.0f, g_vars.visuals_other_modulate_world_color.value.g / 255.0f, g_vars.visuals_other_modulate_world_color.value.b / 255.0f );
            }

            last_world_color = g_vars.visuals_other_modulate_world_color.value;
        }

        last_skybox = g_vars.visuals_other_skybox_selection.value;
        last_modulate_world_enable = g_vars.visuals_other_modulate_world.value;
        last_alive = globals::local_player->alive( );
    }
}

void visuals::render_bar( c_cs_player *player, const entity_box &box, const float &value, const float &progress, color col, bool show ) {
    auto &config = esp_config[ player->index( ) ];
    const auto width = ( box.w * progress );

    if ( player->dormant( ) || progress <= 0.0f ) {
        config.bottom_bar_offset = 0;
        return;
    }

    render::filled_rect( box.x + 1.0f, box.y + box.h + 2.0f + config.bottom_bar_offset, box.w, 2.5f, color{ 46, 46, 46, 110 * opacity_array[ player->index( ) ] } );
    render::filled_rect( box.x + 1.0f, box.y + box.h + 2.0f + config.bottom_bar_offset, width - 2.0f, 4.0f, color{ col, 200 * opacity_array[ player->index( ) ] } );
    render::rect( box.x + 1.0f, box.y + box.h + 2.0f + config.bottom_bar_offset, box.w - 2.0f, 4.0f, color{ 46, 46, 46, 255 * opacity_array[ player->index( ) ] } );

    if ( progress < 1.0f && show )
        render::string( fonts::visuals_04b03, box.x + 1.0f + box.w, box.y + box.h + 2.0f + 7.0f + config.bottom_bar_offset, color{ color::white( ), 255 * opacity_array[ player->index( ) ] }, fmt::format( _xs( "{}" ), value ), false, true );

    config.bottom_bar_offset += 5.5f;
}

void visuals::render_offscreen( c_cs_player *player ) const {
    if ( !g_vars.visuals_other_oof_arrows.value )
        return;

    if ( !globals::local_player->alive( ) || !player->alive( ) )
        return;

    vector_3d forward;
    vector_3d origin, local_origin;

    origin = player->get_render_origin( ), local_origin = globals::local_shoot_pos;

    math::angle_vectors( globals::view_angles, &forward );

    auto angle_to = math::clamp_angle( math::vector_angle( origin - local_origin ) );

    vector_2d origin_screen;

    if ( !render::world_to_screen( origin, origin_screen ) ) {
        auto rotation = ( globals::view_angles - math::vector_angle( local_origin - origin ) ).y - 90.0f;
        auto angle_radians = math::deg_to_rad( rotation );

        auto rotate_point = [ ]( vector_2d center, vector_2d &point, float rotation ) {
            point -= center;
            math::rotate_point( point, rotation );
            point += center;
        };

        const auto distance = g_vars.visuals_other_oof_arrows_radius.value;
        const auto size = g_vars.visuals_other_oof_arrows_size.value;

        auto position = vector_2d(
                ( globals::ui::screen_size.x / 2.0f ) - ( 400 * ( distance / 100.f ) ) * std::cos( angle_radians ),
                ( globals::ui::screen_size.y / 2.0f ) - ( 400 * ( distance / 100.f ) ) * std::sin( angle_radians )
        );

        std::array< vector_2d, 3 > points = {
                vector_2d( position.x + 7.0f + size, position.y + 7.0f + size ),
                vector_2d( position.x - 7.0f - size, position.y ),
                vector_2d( position.x + 7.0f + size, position.y - 7.0f - size ) 
        };

        rotate_point( { position.x, position.y }, points[ 0 ], rotation );
        rotate_point( { position.x, position.y }, points[ 1 ], rotation );
        rotate_point( { position.x, position.y }, points[ 2 ], rotation );

        auto offscreen_color = color{ esp_config[ player->index( ) ].offscreen_color, g_vars.visuals_other_oof_arrows_color.value.a };

        render::filled_triangle( points, offscreen_color );

        if ( g_vars.visuals_other_oof_arrows_outline.value )
            render::triangle( points, color{ offscreen_color, 255 } );
    }
}

void visuals::render_skeleton( c_cs_player *player, matrix_3x4 *bones, color skeleton_color ) const {
    if ( !player->alive( ) || !g_animations.animated_bones[ player->index( ) ].data( ) )
        return;

    if ( !bones )
        bones = g_animations.animated_bones[ player->index( ) ].data( );

    auto model = player->get_model( );

    if ( !model )
        return;

    auto studio_hdr = g_interfaces.model_info->get_studio_model( model );

    if ( !studio_hdr )
        return;

    for ( int n = 0; ( ( n < studio_hdr->numbones ) && ( n < 128 ) ); n++ ) {
        vector_2d bone_position_screen, bone_position_parent_screen;
        vector_3d bone_position, bone_parent_position;

        const auto studio_bone = studio_hdr->bone( n );

        if ( !studio_bone || ( studio_bone->parent < 0 ) || ( studio_bone->parent > ( 128 - 1 ) ) || !( studio_bone->flags & 0x00000100 ) )
            continue;

        math::matrix_position( bones[ n ], bone_position );
        math::matrix_position( bones[ studio_bone->parent ], bone_parent_position );

        if ( render::world_to_screen( bone_position, bone_position_screen ) && render::world_to_screen( bone_parent_position, bone_position_parent_screen ) )
            render::line( bone_position_screen, bone_position_parent_screen, color{ skeleton_color, skeleton_color.a * opacity_array[ player->index( ) ] } );
    }
}