#include "window_setup.hpp"
#include "animations/animations.hpp"
#include "notifications/notifications.hpp"
#include <core/config.hpp>
#include <features/exploits/exploits.hpp>
#include <icons_fa.hpp>

std::shared_ptr< penumbra::window > indicators = nullptr;

void menu::hide_ui( ) {
    if ( !this->is_initialized )
        return;

    if ( this->main_window && this->main_window->is_visible )
        this->main_window->is_visible = false;
}

void menu::handle_hotkeys( ) {
    if ( !this->is_initialized )
        return;

    if ( main_window && main_window->is_visible )
        return;

    globals::hotkeys::thirdperson = g_config.get_hotkey( g_vars.visuals_other_thirdperson_bind, g_vars.visuals_other_thirdperson_toggle.value );
}

void menu::on_screen_size_updated( int width, int height ) {
    if ( penumbra::windows.empty( ) )
        return;

    for ( auto &window : penumbra::windows ) {
        window->position *= glm::vec2{
                std::clamp< float >( globals::ui::screen_size.x / globals::ui::old_screen_size.x, 0.0f, globals::ui::screen_size.x ),
                std::clamp< float >( globals::ui::screen_size.y / globals::ui::old_screen_size.y, 0.0f, globals::ui::screen_size.y ) 
        };
    }

    /* set last screen size. */
    globals::ui::old_screen_size = { static_cast< float >( width ), static_cast< float >( height ) };

    /* clear animation map. */
    animations::clear_map( );
}

void menu::init( ) {
    auto window_center = vector_2d( ( globals::ui::screen_size.x - this->main_window_dimensions.x ) / 2, ( globals::ui::screen_size.y - this->main_window_dimensions.y ) / 2 );

    /* initialize menu. */
    main_window = this->add_window( "Hypnotic", "", nullptr, window_center, this->main_window_dimensions, WINDOW_MAIN );
    {
        auto ragebot = main_window->add_tab( "Ragebot", ICON_FA_CROSSHAIRS, 3 );
        {
            auto general = ragebot->add_child( "General#Ragebot", 0, false );
            {
                general->add_object< penumbra::checkbox >( "Enabled#Aimbot", &g_vars.aimbot_enable.value );
                general->add_object< penumbra::slider< int > >( "Maximum FOV", &g_vars.aimbot_maximum_fov.value, 0, 180, " deg" );
                general->add_object< penumbra::combobox >( "Target selection", &g_vars.aimbot_sort_by.value, std::deque< std::string >{ "Distance", "Field of view" } );
                general->add_object< penumbra::checkbox >( "Automatic fire", &g_vars.aimbot_automatic_shoot.value );
                general->add_object< penumbra::checkbox >( "Silent aim", &g_vars.aimbot_silent.value );
                general->add_object< penumbra::multi_combobox >( "Hitscan", std::deque< std::pair< std::string, bool * > >{ 
                    { "Head", &g_vars.aimbot_hitboxes_head.value },
                    { "Chest", &g_vars.aimbot_hitboxes_chest.value },
                    { "Arms", &g_vars.aimbot_hitboxes_arms.value },
                    { "Stomach", &g_vars.aimbot_hitboxes_stomach.value },
                    { "Legs", &g_vars.aimbot_hitboxes_legs.value } 
                } );

                general->add_object< penumbra::multi_combobox >( "Multi-point", std::deque< std::pair< std::string, bool * > >{
                        { "Head", &g_vars.aimbot_multipoints_head.value },
                        { "Chest", &g_vars.aimbot_multipoints_chest.value },
                        { "Arms", &g_vars.aimbot_multipoints_arms.value },
                        { "Stomach", &g_vars.aimbot_multipoints_stomach.value },
                        { "Legs", &g_vars.aimbot_multipoints_legs.value } 
                } );

                general->add_object< penumbra::slider< int > >( "Point scale", &g_vars.aimbot_multipoint_scale.value, 0, 100, "%" );
                general->add_object< penumbra::slider< int > >( "Hit chance", &g_vars.aimbot_hit_chance.value, 0, 100, "%" );
                general->add_object< penumbra::slider< int > >( "Minimum damage", &g_vars.aimbot_min_damage.value, 0, 100, "%" );
            }
        }

        auto visuals = main_window->add_tab( "Visuals", ICON_FA_USER_EDIT, 3 );
        {
            auto general = visuals->add_child( "Players", 0, false );
            {
                general->add_object< penumbra::checkbox >( "Enabled#Enemy", &g_vars.visual_players_toggled.value );
                general->add_object< penumbra::checkbox >( "Name", &g_vars.visuals_player_name.value )->add_object< penumbra::colorpicker >( "Name color", &g_vars.visuals_name_color.value, false );
                general->add_object< penumbra::checkbox >( "Health", &g_vars.visuals_player_health.value );
                general->add_object< penumbra::checkbox >( "Custom health color", &g_vars.visuals_player_health_override.value )->add_object< penumbra::colorpicker >( "Health bar color", &g_vars.visuals_health_override_color.value, false );
                general->add_object< penumbra::checkbox >( "Box", &g_vars.visuals_player_box.value )->add_object< penumbra::colorpicker >( "Box color", &g_vars.visuals_box_color.value, false );
                general->add_object< penumbra::checkbox >( "Weapon name", &g_vars.visuals_player_weapon.value );
                general->add_object< penumbra::checkbox >( "Skeleton", &g_vars.visuals_player_skeleton.value )->add_object< penumbra::colorpicker >( "Skeleton color", &g_vars.visuals_skeleton_color.value, false );
                general->add_object< penumbra::checkbox >( "Skeleton History", &g_vars.visuals_player_skeleton_history.value )->add_object< penumbra::colorpicker >( "Skeleton history color", &g_vars.visuals_skeleton_history_color.value, false );

                general->add_object< penumbra::multi_combobox >( "Flags", std::deque< std::pair< std::string, bool * > >{
                    { "Armor", &g_vars.visuals_player_flags_armor.value },
                    { "Distance", &g_vars.visuals_player_distance.value },
                    { "Bot", &g_vars.visuals_player_flags_bot.value },
                    { "Scoped", &g_vars.visuals_player_flags_scoped.value },
                    { "Lag", &g_vars.visuals_player_flags_lag_amount.value } 
                } );

                general->add_object< penumbra::checkbox >( "Offscreen arrows", &g_vars.visuals_other_oof_arrows.value )->add_object< penumbra::colorpicker >( "Offscreen arrows color", &g_vars.visuals_other_oof_arrows_color.value );
                general->add_object< penumbra::slider< int > >( "Size", &g_vars.visuals_other_oof_arrows_size.value, 0.f, 100.f, "%" );
                general->add_object< penumbra::slider< int > >( "Distance", &g_vars.visuals_other_oof_arrows_radius.value, 0.f, 100.f, "%" );
                general->add_object< penumbra::checkbox >( "Outline", &g_vars.visuals_other_oof_arrows_outline.value );    
            }

            auto models = visuals->add_child( "Models", 1, false );
            {
                models->add_object< penumbra::checkbox >( "Enabled#Enemy Chams", &g_vars.visuals_render_player_chams_toggle.value );
                models->add_object< penumbra::checkbox >( "Visible", &g_vars.visuals_render_player_chams.value )->add_object< penumbra::colorpicker >( 
                    "Enemy model color", &g_vars.visuals_render_player_chams_color.value, true 
                );

                models->add_object< penumbra::checkbox >( "Invisible", &g_vars.visuals_render_player_chams_xqz.value )->add_object< penumbra::colorpicker >( 
                    "Enemy model XQZ color", &g_vars.visuals_render_player_chams_xqz_color.value, true
                );

                models->add_object< penumbra::checkbox >( "History", &g_vars.visuals_render_player_chams_lag_record.value )->add_object< penumbra::colorpicker >(
                    "Lag record color", &g_vars.visuals_render_player_chams_lag_record_color.value, true 
                );

                models->add_object< penumbra::combobox >( "Material", &g_vars.visuals_render_player_chams_material.value, std::deque< std::string >{ "Flat", "Textured" } );
                models->add_object< penumbra::checkbox >( "Glow", &g_vars.visuals_render_player_glow.value )->add_object< penumbra::colorpicker >( "Glow color", &g_vars.visuals_render_player_glow_color.value, true );
            }

            auto world = visuals->add_child( "World", 1, false );
            {
                world->add_object< penumbra::checkbox >( "World modulation", &g_vars.visuals_other_modulate_world.value )->add_object< penumbra::colorpicker >( "Modulation color", &g_vars.visuals_other_modulate_world_color.value, false );
                world->add_object< penumbra::checkbox >( "Transparent props", &g_vars.visuals_other_transparent_props.value );
                world->add_object< penumbra::slider< float > >( "Value#TPP", &g_vars.visuals_other_prop_transparency.value, 0.0f, 100.0f, "%" );
                world->add_object< penumbra::checkbox >( "Skybox changer", &g_vars.visuals_other_skybox_changer.value );
                world->add_object< penumbra::combobox >( "Sky name", &g_vars.visuals_other_skybox_selection.value, std::deque< std::string >
                {
                    XOR( "cs_baggage_skybox" ),
                    XOR( "cs_tibet" ),
                    XOR( "vietnam" ),
                    XOR( "sky_lunacy" ),
                    XOR( "embassy" ),
                    XOR( "italy" ),
                    XOR( "jungle" ),
                    XOR( "office" ),
                    XOR( "sky_cs15_daylight01_hdr" ),
                    XOR( "sky_cs15_daylight02_hdr" ),
                    XOR( "sky_day02_05" ),
                    XOR( "sky_csgo_cloudy01" ),
                    XOR( "sky_csgo_night02" ),
                    XOR( "sky_csgo_night02b" ),
                } );
            }

            auto misc = visuals->add_child( "Misc#visuals", 2, false );
            {
                 misc->add_object< penumbra::slider< float > >( "Camera FOV", &g_vars.visuals_other_fov.value, 60.f, 180.f, " FOV" );

                misc->add_object< penumbra::checkbox >( "Override view model", &g_vars.visuals_other_viewmodel_override.value );
                misc->add_object< penumbra::slider< float > >( "Field of view", &g_vars.visuals_other_viewmodel_fov.value, -180.f, 180.f, " FOV" );
                misc->add_object< penumbra::slider< float > >( "Forward#VM", &g_vars.visuals_other_viewmodel_offset.value.x, -180.f, 180.f, " deg" );
                misc->add_object< penumbra::slider< float > >( "Right#VM", &g_vars.visuals_other_viewmodel_offset.value.y, -180.f, 180.f, " deg" );
                misc->add_object< penumbra::slider< float > >( "Up#VM", &g_vars.visuals_other_viewmodel_offset.value.z, -180.f, 180.f, " deg" );
                misc->add_object< penumbra::slider< float > >( "Roll#VM", &g_vars.visuals_other_viewmodel_roll.value, -180.f, 180.f, " deg" );

                auto thirdperson = misc->add_object< penumbra::checkbox >( "Thirdperson", &g_vars.visuals_other_thirdperson_enabled.value );
                {
                    thirdperson->add_object< penumbra::hotkey >( "Thirdperson hotkey", &g_vars.visuals_other_thirdperson_bind.value, &g_vars.visuals_other_thirdperson_toggle.value );
                }

                misc->add_object< penumbra::slider< float > >( "Distance#third_person_camera", &g_vars.visuals_other_thirdperson_distance.value, 0.0f, 230.0f, " %" );

                misc->add_object< penumbra::multi_combobox >( "Removals", std::deque< std::pair< std::string, bool * > >
                { 
                    { "Viewmodel bob", &g_vars.visuals_other_remove_view_bob.value },
                    { "Scope overlay", &g_vars.visuals_other_remove_scope_overlay.value },
                    { "Post processing", &g_vars.visuals_other_remove_post_processing.value },
                    { "No visual punch", &g_vars.visuals_other_remove_aim_punch.value },
                    { "No visual kick", &g_vars.visuals_other_remove_view_punch.value },
                    { "Smoke", &g_vars.visuals_other_remove_smoke.value },
                    { "Flash overlay", &g_vars.visuals_other_remove_flash_overlay.value }
                } );

                misc->add_object< penumbra::checkbox >( "Force viewmodel while scoped", &g_vars.visuals_other_force_viewmodel_in_scope.value );
                misc->add_object< penumbra::checkbox >( "Disable model occlusion", &g_vars.visuals_other_disable_model_occlusion.value );
                misc->add_object< penumbra::checkbox >( "Show server hitboxes", &g_vars.visuals_other_show_server_hitboxes.value );
            }
        }

        auto miscellaneous = main_window->add_tab( "Miscellaneous", ICON_FA_COGS, 3 );
        {
            auto misc = miscellaneous->add_child( "Other#Miscellaneous", 0, false );
            {
                misc->add_object< penumbra::checkbox >( "Bypass sv_pure", &g_vars.misc_bypass_sv_pure.value );
                misc->add_object< penumbra::checkbox >( "Auto bunny hop", &g_vars.misc_bunny_hop.value );
                misc->add_object< penumbra::combobox >( "Auto strafer", &g_vars.misc_auto_strafe_type.value, std::deque< std::string >{
                    "None",
                    "Mouse",
                    "Movement keys" 
                } );

                misc->add_object< penumbra::multi_combobox >( "Log events", std::deque< std::pair< std::string, bool * > >{ 
                    { "Planted C4", &g_vars.misc_events_log_bomb.value },
                    { "Player hurt", &g_vars.misc_events_log_damage.value }, 
                    { "Purchased weapons", &g_vars.misc_events_log_buy.value } 
                } );

                misc->add_object< penumbra::checkbox >( "Hitmarker", &g_vars.misc_hitmarker.value );
                misc->add_object< penumbra::button >( "Load test config", [ & ] {
                    g_interfaces.engine_client->client_cmd_unrestricted( "sv_cheats 1; bot_stop 1; bot_kick all; bot_dont_shoot 1; mp_warmuptime 99999; mp_warmup_start" );
                    g_notify.add( "Loaded test game config" );
                } );
            }

            auto configuration = miscellaneous->add_child( "Config", 1, false );
            {
                auto ui_color = configuration->add_object< penumbra::label >( "Theme" );
                {
                    ui_color->add_object< penumbra::colorpicker >( "menu_controls_color", &g_vars.ui_theme.value, false );
                }

                config_list = configuration->add_object< penumbra::listbox >( "Select config", &this->config_selected, this->config_items, true, 150.0f );

                auto config_text = configuration->add_object< penumbra::textbox >( "Name", &config_name, 100 );

                configuration->add_object< penumbra::button >( "Save", [ & ] {
                    if ( !config_list ) return;

                    this->mutex.lock( );

                    g_config.save( config_name );

                    this->refresh_config_items( );
                    this->mutex.unlock( );
                } );

                configuration->add_object< penumbra::button >( "Load", [ & ] {
                    if ( !config_list || config_list->items.empty( ) ) return;

                    this->mutex.lock( );
                    g_config.load( config_name );
                    // this->refresh_config_items( );

                    this->mutex.unlock( );
                } );

                configuration->add_object< penumbra::button >( "Delete", [ & ] {
                    if ( !config_list || config_list->items.empty( ) ) return;

                    this->mutex.lock( );
                    //
                    g_config.remove( config_name );
                    this->refresh_config_items( );

                    if ( !config_list->items.empty( ) ) {
                        this->config_name = config_list->items.back( );
                        this->config_selected = config_list->items.size( ) - 1;
                    }

                    this->mutex.unlock( );
                } );

                configuration->add_object< penumbra::button >( "Refresh list", [ & ] {
                    this->mutex.lock( );
                    this->refresh_config_items( );

                    if ( !config_list->items.empty( ) )
                        this->config_name = config_list->items.back( );

                    this->config_selected = 0;
                    this->mutex.unlock( );
                } );
            }
 
        }

        auto scripts = main_window->add_tab( "Scripts", ICON_FA_SCROLL, 3 );
        {
        }
    }

    /* load config list. */
    this->refresh_config_items( );
}

void menu::refresh_config_items( ) {
    if ( !config_list )
        return;

    auto sanitize_name = []( const std::string &dir ) {
        const auto dot = dir.find_last_of( "." );
        return dir.substr( 0, dot );
    };

    config_list->clear( );

    for ( const auto &entry : std::filesystem::recursive_directory_iterator( g_config.folder_path ) ) {
        if ( !entry.exists( ) || HASH( entry.path( ).extension( ).string( ).c_str( ) ) != HASH_CT( ".json" ) )
            continue;

        auto name = sanitize_name( entry.path( ).filename( ).string( ) );

        config_list->add_item( name );
    }
}

int last_selected_config = 0;

void menu::render( ) {
    this->is_initialized = true;

    using namespace penumbra;

    if ( !this->is_initialized )
        return;

    //if ( !penumbra::focus_items.empty( ) ) { /* was making a better focusing system but ehhh. */
    //    for ( auto& item : penumbra::focus_items ) {
    //        auto object = reinterpret_cast< penumbra::object * >( item );

    //        if ( !object ) continue;

    //        if ( object->opened )
    //            penumbra::focused = object;
    //    }
    //}
  
    this->handle_hotkeys( );

    if ( main_window && input::key_pressed( VK_INSERT ) )
        main_window->is_visible = !main_window->is_visible;

    if ( !windows.empty( ) ) {
        for ( auto &window : penumbra::windows ) {
            const auto animation = animations::get( HASH_CT( "menu_main" ) + HASH( window->name.c_str( ) ), 0.0f );

            /* toggle animation. */
            animations::lerp_to( HASH_CT( "menu_main" ) + HASH( window->name.c_str( ) ), window->is_visible, 0.10f );
            penumbra::globals::fade_opacity[ window.get( ) ] = std::clamp< float >( animation.value, 0.0f, 1.0f );

            auto should_close = penumbra::globals::fade_opacity[ window.get( ) ] <= 0.05f;

            window->should_disable_input = should_close;

            if ( should_close ) {
                //if ( !this->is_visible )
                //    animations::clear_map( );

                continue;
            }

            window->input( );
            window->paint( );
        }

        std::ranges::sort( penumbra::windows, [ & ]( const std::shared_ptr< penumbra::window > &a, const std::shared_ptr< penumbra::window > &b ) -> bool {
            return !a->is_hovered_in_rectangle && a->last_click_time < b->last_click_time;
        } );
    }

    if ( last_selected_config != this->config_selected ) {
        config_name = config_list->items[ *config_list->value ];

        /* update selected config. */
        last_selected_config = this->config_selected;
    }

    penumbra::globals::theme_accent = g_vars.ui_theme.value;
}

void menu::display_watermark( ) {
}

bool menu::is_opened( ) {
    if ( !main_window )
        return false;

    return main_window->is_visible;
}