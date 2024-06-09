#include "visuals.hpp"
#include <features/aimbot/aimbot.hpp>

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

        if ( opacity_array.empty( ) )
            continue;

        auto network_name = HASH( entity->get_client_class( )->network_name );

        switch ( network_name ) {
            case HASH_CT( "CCSPlayer" ): {
                const auto player = entity->get< c_cs_player * >( );

                if ( !player || player == globals::local_player || player->team( ) == globals::local_player->team( ) )
                    break;

                if ( g_vars.visual_players_toggled.value )
                    render_player( player );

                // player->draw_server_hitboxes( );
            } break;
            default:
                break;
        }
    }

    render_hitmarker( );
}

void visuals::render_hitmarker( ) {
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

void visuals::animate( c_cs_player *player ) {
    if ( !player->alive( ) && opacity_array[ player->index( ) ] != 0.0f )
        opacity_array[ player->index( ) ] -= std::clamp< float >( 1.0f * ( ( 1.0f / 0.4f ) * ImGui::GetIO( ).DeltaTime ), 0.0f, 1.0f );

    else
        opacity_array[ player->index( ) ] = 1.0f;
}

void visuals::render_player( c_cs_player *player ) {
    animate( player );

    if ( opacity_array[ player->index( ) ] <= 0.02f )
        return;

    player_info_t player_info;
    g_interfaces.engine_client->get_player_info( player->index( ), &player_info );

    render_offscreen( player, player_info );

    if ( player->dormant( ) )
        return;

    box player_box;

    if ( player->compute_bounding_box( player_box ) ) {
        if ( g_vars.visuals_player_name.value ) {
            auto name = std::string( player_info.name );
            auto name_dimensions = render::get_text_size( fonts::visuals_segoe_ui, name );

            render::string( fonts::visuals_segoe_ui, player_box.x + ( player_box.w / 2 ) - name_dimensions.x / 2, player_box.y - name_dimensions.y - 2.0f, color{ color::white( ), 240 * opacity_array[ player->index( ) ] }, name, true );
        }

        if ( g_vars.visuals_player_box.value ) {
            render::rect( player_box.x + 1, player_box.y + 1, player_box.w - 2, player_box.h - 2, color{ 46, 46, 46, 160 * opacity_array[ player->index( ) ] } );
            render::rect( player_box.x - 1, player_box.y - 1, player_box.w + 2, player_box.h + 2, color{ 46, 46, 46, 160 * opacity_array[ player->index( ) ] } );
            render::rect( player_box.x, player_box.y, player_box.w, player_box.h, color{ g_vars.visuals_box_color.value, 200 * opacity_array[ player->index( ) ] } );
        }

        auto &log = g_animations.lag_info[ player->index( ) ];

        if ( g_vars.visuals_player_skeleton_history.value && !log.lag_records.empty( ) ) {
            
            for ( auto &record : log.lag_records ) {
                if ( !record ) continue;

                render_skeleton( player, record, g_vars.visuals_skeleton_history_color.value );
            }
        }

        if ( g_vars.visuals_player_skeleton.value && !log.anim_records.empty( ) ) {
            auto current_record = &log.anim_records.front( );

            render_skeleton( player, current_record, g_vars.visuals_skeleton_history_color.value );
        }

        if ( g_vars.visuals_player_health.value ) {
            auto health = player->health( );

            if ( health > 100 )
                health = 100;

            auto health_bar_delta = static_cast< int >( health * player_box.h / 100 );
            auto health_text = fmt::format( "{}", health );
            auto health_text_dim = render::get_text_size( fonts::visuals_04b03, health_text );
            auto health_bar_color = g_vars.visuals_player_health_override.value ? color{ g_vars.visuals_health_override_color.value, 255 * opacity_array[ player->index( ) ] } : color{ 166, 0, 0, 255 * opacity_array[ player->index( ) ] }.lerp( color{ 157, 255, 0, 255 * opacity_array[ player->index( ) ] }, std::clamp< float >( static_cast< float >( player->health( ) ) / 100.f, 0.0f, 1.0f ) );

            render::filled_rect( player_box.x - 6.5f, player_box.y, 2.5f, player_box.h, color{ 46, 46, 46, 110 * opacity_array[ player->index( ) ] } );
            render::filled_rect( player_box.x - 6.5f, std::clamp< float >( player_box.y - 1 + player_box.h - health_bar_delta, player_box.y, player_box.y + player_box.h ), 2.5f, health_bar_delta, color{ health_bar_color, 200 * opacity_array[ player->index( ) ] } );
            render::rect( player_box.x - 7.5f, player_box.y - 1, 4.f, player_box.h + 2, color{ 46, 46, 46, 190 * opacity_array[ player->index( ) ] } );

            if ( health > 0 && health < 100 )
                render::string( fonts::visuals_04b03, player_box.x - 15.5f + ( health_text_dim.x / 2 ), player_box.y - 4 + player_box.h - health_bar_delta, color{ color::white( ), 240 * opacity_array[ player->index( ) ] }, health_text, true );
        }

        if ( g_vars.visuals_player_weapon.value ) {
            auto weapon = g_interfaces.entity_list->get_client_entity_from_handle< c_cs_weapon_base * >( player->weapon_handle( ) );

            if ( weapon ) {
                auto weapon_data = weapon->get_weapon_data( );

                if ( weapon_data ) {
                    auto weapon_text = utils::convert_utf8( weapon->get_name( ) );
                    auto text_dimensions = render::get_text_size( fonts::visuals_04b03, weapon_text );

                    std::transform( weapon_text.begin( ), weapon_text.end( ), weapon_text.begin( ), ::toupper );

                    render::string( fonts::visuals_04b03, player_box.x + ( player_box.w / 2 ) - text_dimensions.x / 2, player_box.y + player_box.h + 1.5f, color{ color::white( ), 240 * opacity_array[ player->index( ) ] }, weapon_text, true );
                }
            }
        }

        std::deque< std::pair< std::string, color > > flags;

        {
            auto distance_to_player = glm::length( player->origin( ) - globals::local_player->origin( ) ) * 0.01905f;
            auto distance_to_player_text = fmt::format( "{}M", static_cast< int >( std::roundf( distance_to_player ) ) );
            auto text_dimensions = render::get_text_size( fonts::visuals_04b03, distance_to_player_text );

            if ( g_vars.visuals_player_distance.value )
                flags.push_back( { distance_to_player_text, { 255, 255, 255 } } );

            if ( g_vars.visuals_player_flags_bot.value && player_info.fake_player )
                flags.push_back( { "BOT", { 255, 255, 255 } } );

            if ( g_vars.visuals_player_flags_scoped.value && player->scoped( ) )
                flags.push_back( { "SCOPED", { 255, 255, 255 } } );

            auto get_armor_type = [ ]( c_cs_player *player ) -> std::string {
                if ( player->armor( ) > 0 && player->helmet( ) )
                    return "HK";

                else if ( player->armor( ) )
                    return "K";

                else if ( player->helmet( ) )
                    return "H";

                return "";
            };

            if ( g_vars.visuals_player_flags_money.value && player->account( ) > 0 ) {
                auto balence = fmt::format( "${}", player->account( ) );

                flags.push_back( { balence, { 255, 255, 255 } } );
            }

            if ( g_vars.visuals_player_flags_armor.value )
                flags.push_back( { get_armor_type( player ), { 255, 255, 255 } } );
        }

        for ( size_t i = { 0ul }; i < flags.size( ); ++i ) {
            auto &flag_object = flags.at( i );
            auto offset = ( i * ( render::get_text_size( fonts::visuals_04b03, flags[ i ].first ).y + 1.0f ) );

            render::string( fonts::visuals_04b03, player_box.x + player_box.w + 2, player_box.y + offset - 1, color{ color::white( ), 240 * opacity_array[ player->index( ) ] }, flag_object.first, true );
        }
    }
}

void visuals::world_modulation( ) {
    static auto last_world_color = g_vars.visuals_other_modulate_world_color.value;
    static auto last_alive = false;
    static auto last_skybox = g_vars.visuals_other_skybox_selection.value;
    static auto last_modulate_world_enable = g_vars.visuals_other_modulate_world.value;

    if ( last_alive != globals::local_player->alive( ) || last_world_color.r != g_vars.visuals_other_modulate_world_color.value.r || last_world_color.g != g_vars.visuals_other_modulate_world_color.value.g || last_world_color.b != g_vars.visuals_other_modulate_world_color.value.b || last_skybox != g_vars.visuals_other_skybox_selection.value || last_modulate_world_enable != g_vars.visuals_other_modulate_world.value ) {
        static auto load_named_sky = signature::find( "engine.dll", XOR( "55 8B EC 81 ? ? ? ? ? 56 57 8B F9 C7" ) ).get< bool( __thiscall * )( const char * ) >( );
        load_named_sky( globals::sky_names[ g_vars.visuals_other_skybox_selection.value ] );
        last_skybox = g_vars.visuals_other_skybox_selection.value;

        if ( g_vars.visuals_other_modulate_world.value ) {
            for ( uint16_t h = g_interfaces.material_system->first_material( ); h != g_interfaces.material_system->invalid_material( ); h = g_interfaces.material_system->next_material( h ) ) {
                auto mat = g_interfaces.material_system->get_material( h );

                if ( !mat )
                    continue;

                if ( HASH( mat->get_texture_group_name( ) ) == HASH_CT( "World textures" ) && g_vars.visuals_other_modulate_world_color.value != color( 255, 255, 255 ) )
                    mat->color_modulate( g_vars.visuals_other_modulate_world_color.value.r / 255.0f, g_vars.visuals_other_modulate_world_color.value.g / 255.0f, g_vars.visuals_other_modulate_world_color.value.b / 255.0f );
            }
        }

        last_world_color = g_vars.visuals_other_modulate_world_color.value;
        last_skybox = g_vars.visuals_other_skybox_selection.value;
        last_modulate_world_enable = g_vars.visuals_other_modulate_world.value;
        last_alive = globals::local_player->alive( );
    }
}

void visuals::render_offscreen( c_cs_player *player, const player_info_t &player_info ) const {
    if ( !g_vars.visuals_other_oof_arrows.value )
        return;

    if ( !player->alive( ) || player->dormant( ) )
        return;

    if ( g_animations.lag_info[ player->index( ) ].anim_records.empty( ) )
        return;

    auto &log = g_animations.lag_info[ player->index( ) ].anim_records.front( );

    vector_2d origin_screen;
    vector_3d origin, local_origin;

    origin = log.origin, local_origin = globals::local_player->origin( );

    vector_3d forward = { };
    math::angle_vectors( globals::view_angles, &forward );

    auto origin_lerped = math::lerp_vector( local_origin, globals::shoot_position, forward.y );

    auto angle_to = math::clamp_angle( math::vector_angle( origin - globals::shoot_position ) );

    if ( !render::world_to_screen( origin, origin_screen ) ) {
        auto rotation = ( globals::view_angles - math::vector_angle( origin_lerped - origin ) ).y - 90.0f;
        auto angle_radians = math::deg_to_rad( rotation );

        auto rotate_point = []( vector_2d center, vector_2d &point, float rotation ) {
            point -= center;
            math::rotate_point( point, rotation );
            point += center;
        };

        const auto distance = g_vars.visuals_other_oof_arrows_radius.value;
        const auto size = g_vars.visuals_other_oof_arrows_size.value;

        auto position = vector_2d(
                ( globals::ui::screen_size.x / 2.0f ) - ( 400 * ( distance / 100.f ) ) * std::cosf( angle_radians ),
                ( globals::ui::screen_size.y / 2.0f ) - ( 400 * ( distance / 100.f ) ) * std::sinf( angle_radians ) 
        );

        std::array< vector_2d, 3 > points = {
                vector_2d( position.x + 7.0f + size, position.y + 7.0f + size ),
                vector_2d( position.x - 7.0f - size, position.y ),
                vector_2d( position.x + 7.0f + size, position.y - 7.0f - size )
        };

        rotate_point( { position.x, position.y }, points[ 0 ], rotation );
        rotate_point( { position.x, position.y }, points[ 1 ], rotation );
        rotate_point( { position.x, position.y }, points[ 2 ], rotation );

        auto offscreen_color = color{ g_vars.visuals_other_oof_arrows_color.value, g_vars.visuals_other_oof_arrows_color.value.a };

        render::filled_triangle( points, offscreen_color );

        if ( g_vars.visuals_other_oof_arrows_outline.value )
            render::triangle( points, color{ offscreen_color, 255 } );
    }
}

void visuals::render_skeleton( c_cs_player *player, lag_record *record, color skeleton_color ) const {
    if ( !player->alive( ) )
        return;

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

        math::matrix_position( record->bones[ n ], bone_position );
        math::matrix_position( record->bones[ studio_bone->parent ], bone_parent_position );

        if ( render::world_to_screen( bone_position, bone_position_screen ) && render::world_to_screen( bone_parent_position, bone_position_parent_screen ) )
            render::line( bone_position_screen, bone_position_parent_screen, color{ color::white( ), skeleton_color.a * opacity_array[ player->index( ) ] } );
    }
}