#pragma once

#define MAX_STRUCT_FIELD_SIZE 64
#define CONFIG_TYPE_IMPL( type )                                                                                                   \
    class config_##type {                                                                                                          \
    public:                                                                                                                        \
        config_##type( ) {}                                                                                                        \
        config_##type( int value_type, const char *name, type value ) : value_type( value_type ), name( name ), value( value ){ }; \
                                                                                                                                   \
        int value_type;                                                                                                            \
        const char *name;                                                                                                          \
        __declspec( align( MAX_STRUCT_FIELD_SIZE ) ) type value;                                                                   \
    };                                                                                                                             \
                                                                                                                                   \
    constexpr int config_type_##type = __LINE__;                                                                                   \
    static_assert( sizeof( config_##type ) == sizeof( config_bool ), #type " option size doesn't match uniform option size" )

using namespace std;

CONFIG_TYPE_IMPL( bool );
CONFIG_TYPE_IMPL( int );
CONFIG_TYPE_IMPL( float );
CONFIG_TYPE_IMPL( color );
CONFIG_TYPE_IMPL( double );
CONFIG_TYPE_IMPL( string );
CONFIG_TYPE_IMPL( vector_3d );

#define CONFIG_VARIABLE( name, type, value ) \
    config_##type name = config_##type { config_type_##type, #name, value }

enum strafe_type : int {
    view_angles = 1,
    movement_keys
};

enum color_type : int {
    team_based = 0,
    custom
};

namespace config
{
#pragma( pack, 1 )
    class variables {
    public:
        CONFIG_VARIABLE( ui_theme, color, color( 176, 115, 255 ) );
        CONFIG_VARIABLE( animation_speed, float, 1.0f );

#pragma region ragebot
        CONFIG_VARIABLE( aimbot_enable, bool, false );
        CONFIG_VARIABLE( aimbot_fix_fakelag, bool, false );
        CONFIG_VARIABLE( aimbot_delay_shot, bool, false );
        CONFIG_VARIABLE( aimbot_silent, bool, false );
        CONFIG_VARIABLE( aimbot_auto_stop, bool, true );
        CONFIG_VARIABLE( aimbot_sort_by, int, 1 );
        CONFIG_VARIABLE( aimbot_maximum_fov, int, 180 );
        CONFIG_VARIABLE( aimbot_automatic_shoot, bool, false );
        CONFIG_VARIABLE( aimbot_resolver, bool, false );
        CONFIG_VARIABLE( aimbot_resolver_override, bool, false );
        CONFIG_VARIABLE( aimbot_hitboxes_head, bool, false );
        CONFIG_VARIABLE( aimbot_hitboxes_chest, bool, false );
        CONFIG_VARIABLE( aimbot_hitboxes_stomach, bool, false );
        CONFIG_VARIABLE( aimbot_hitboxes_pelvis, bool, false );
        CONFIG_VARIABLE( aimbot_hitboxes_arms, bool, false );
        CONFIG_VARIABLE( aimbot_hitboxes_legs, bool, false );

        CONFIG_VARIABLE( aimbot_multipoints_head, bool, false );
        CONFIG_VARIABLE( aimbot_multipoints_chest, bool, false );
        CONFIG_VARIABLE( aimbot_multipoints_stomach, bool, false );
        CONFIG_VARIABLE( aimbot_multipoints_pelvis, bool, false );
        CONFIG_VARIABLE( aimbot_multipoints_arms, bool, false );
        CONFIG_VARIABLE( aimbot_multipoints_legs, bool, false );
        CONFIG_VARIABLE( aimbot_multipoints_dynamic_scale, bool, false );

        CONFIG_VARIABLE( aimbot_multipoint_scale, int, 65 );
        CONFIG_VARIABLE( aimbot_min_damage, int, 32 );
        CONFIG_VARIABLE( aimbot_hit_chance, int, 60 );
#pragma endregion

#pragma region exploits
        CONFIG_VARIABLE( exploits_antiaim_static_legs_in_air, bool, false );
        CONFIG_VARIABLE( exploits_antiaim, bool, false );
        CONFIG_VARIABLE( exploits_antiaim_auto_direction, bool, false );
        CONFIG_VARIABLE( exploits_antiaim_manual, bool, false );
        CONFIG_VARIABLE( exploits_antiaim_distortion, bool, false );
        CONFIG_VARIABLE( exploits_antiaim_distortion_speed, float, 0.0f ); 
        CONFIG_VARIABLE( exploits_antiaim_distortion_range, float, 0.0f );

        CONFIG_VARIABLE( exploits_antiaim_leg_movement, int, 0 );
        CONFIG_VARIABLE( exploits_antiaim_pitch_type, int, 0 );
        CONFIG_VARIABLE( exploits_antiaim_dir_type, int, 0 );
        CONFIG_VARIABLE( exploits_antiaim_yaw_type, int, 0 );
        CONFIG_VARIABLE( exploits_antiaim_fake, bool, false );
        CONFIG_VARIABLE( exploits_antiaim_lby_break, bool, false );
        CONFIG_VARIABLE( exploits_antiaim_yaw_offset, float, 0.f );
        CONFIG_VARIABLE( exploits_antiaim_lby_break_delta, float, 0.f );
        CONFIG_VARIABLE( exploits_antiaim_fake_yaw_type, int, 0 );
        CONFIG_VARIABLE( exploits_antiaim_manual_left, int, 0 );
        CONFIG_VARIABLE( exploits_antiaim_manual_right, int, 0 );
        CONFIG_VARIABLE( exploits_antiaim_manual_back, int, 0 );
        CONFIG_VARIABLE( exploits_antiaim_manual_show_indicators, bool, false );
        CONFIG_VARIABLE( exploits_antiaim_manual_indicators_color, color, color( 176, 115, 255, 150 ) );
        CONFIG_VARIABLE( exploits_antiaim_manual_show_indicators_outline, bool, true );
        CONFIG_VARIABLE( exploits_antiaim_placeholder, int, 0 );
        CONFIG_VARIABLE( exploits_antiaim_range, float, 0.f );
        CONFIG_VARIABLE( exploits_antiaim_spin_speed, float, 0.f );
        CONFIG_VARIABLE( exploits_fakelag, bool, false );
        CONFIG_VARIABLE( exploits_fakelag_limit, int, 15 );
#pragma endregion

#pragma region visuals
        CONFIG_VARIABLE( visual_players_toggled, bool, false );
        CONFIG_VARIABLE( visual_buildings_toggled, bool, false );

        CONFIG_VARIABLE( visuals_buildings_box_color, color, color( 128, 0, 255 ) );
        CONFIG_VARIABLE( visuals_buildings_name_color, color, color( 255, 255, 255 ) );
        CONFIG_VARIABLE( visuals_buildings_health_override_color, color, color( 255, 255, 255 ) );
        CONFIG_VARIABLE( visuals_buildings_render_chams_color, color, color( 231, 110, 255 ) );
        CONFIG_VARIABLE( visuals_buildings_render_chams_xqz_color, color, color( 255, 110, 117 ) );
        CONFIG_VARIABLE( visuals_buildings_render_chams_highlight, color, color( 255, 255, 255 ) );

        // Colors.
        CONFIG_VARIABLE( visuals_box_color, color, color( 128, 0, 255 ) );
        CONFIG_VARIABLE( visuals_name_color, color, color( 255, 255, 255 ) );
        CONFIG_VARIABLE( visuals_skeleton_color, color, color( 255, 255, 255 ) );
        CONFIG_VARIABLE( visuals_lby_timer_color, color, color( 255, 77, 252 ) );
        CONFIG_VARIABLE( visuals_skeleton_history_color, color, color( 255, 255, 255 ) );
        CONFIG_VARIABLE( visuals_health_override_color, color, color( 255, 255, 255 ) );
        CONFIG_VARIABLE( visuals_highlight_target_color, color, color( 255, 153, 0 ) );
        CONFIG_VARIABLE( visuals_render_player_chams_color, color, color( 231, 110, 255 ) );
        CONFIG_VARIABLE( visuals_render_player_chams_xqz_color, color, color( 255, 110, 117 ) );
        CONFIG_VARIABLE( visuals_render_player_chams_highlight, color, color( 255, 255, 255 ) );

        // Basic features.
        CONFIG_VARIABLE( visuals_player_name, bool, true );
        CONFIG_VARIABLE( visuals_player_distance, bool, true );
        CONFIG_VARIABLE( visuals_player_box, bool, true );
        CONFIG_VARIABLE( visuals_player_lby_timer, bool, false );
        CONFIG_VARIABLE( visuals_player_highlight_target, bool, true );
        CONFIG_VARIABLE( visuals_player_highlight_steam_friend, bool, false );
        CONFIG_VARIABLE( visuals_player_health, bool, true );
        CONFIG_VARIABLE( visuals_player_skeleton, bool, false );
        CONFIG_VARIABLE( visuals_player_skeleton_history, bool, false );
        CONFIG_VARIABLE( visuals_player_weapon_text, bool, false );
        CONFIG_VARIABLE( visuals_player_weapon_icon, bool, false );
        CONFIG_VARIABLE( visuals_player_health_override, bool, false );
        CONFIG_VARIABLE( visuals_player_filled_box_transparency, float, 60.0f );
        CONFIG_VARIABLE( visuals_render_if_dormant, bool, false );

        CONFIG_VARIABLE( visuals_player_flags_scoped, bool, false );
        CONFIG_VARIABLE( visuals_player_flags_armor, bool, false );
        CONFIG_VARIABLE( visuals_player_flags_money, bool, false );
        CONFIG_VARIABLE( visuals_player_flags_lag_amount, bool, false );
        CONFIG_VARIABLE( visuals_player_flags_bot, bool, false );
        CONFIG_VARIABLE( visuals_player_flags_steam_friend, bool, false );
        CONFIG_VARIABLE( visuals_other_fov, float, 90.0f );
        CONFIG_VARIABLE( visuals_other_scoped_fov, float, 90.0f );
        CONFIG_VARIABLE( visuals_render_player_glow, bool, false );
        CONFIG_VARIABLE( visuals_render_player_glow_color, color, color( 162, 56, 255, 180 ) );
        CONFIG_VARIABLE( visuals_other_modulate_world_color, color, color( 255, 255, 255, 255 ) );
        CONFIG_VARIABLE( visuals_other_skybox_changer, bool, false );
        CONFIG_VARIABLE( visuals_other_fullbright, bool, false );
        CONFIG_VARIABLE( visuals_other_modulate_world, bool, false );
        CONFIG_VARIABLE( visuals_other_penetration_crosshair, bool, false );
        CONFIG_VARIABLE( visuals_other_skybox_selection, int, 0 );
        CONFIG_VARIABLE( visual_weapon_dropped_name, bool, false );
        CONFIG_VARIABLE( visual_weapon_dropped_box, bool, false );
        CONFIG_VARIABLE( visual_weapon_dropped_ammo, bool, false );
        CONFIG_VARIABLE( visual_weapon_dropped_ammo_color, color, color( 255, 77, 252 ) );

        CONFIG_VARIABLE( visuals_other_viewmodel_fov, float, 60.0f );
        CONFIG_VARIABLE( visuals_other_viewmodel_override, bool, false );
        CONFIG_VARIABLE( visuals_other_viewmodel_offset, vector_3d, vector_3d( 0.f, 0.f, 0.f ) );
        CONFIG_VARIABLE( visuals_other_viewmodel_roll, float, 0.0f );
        CONFIG_VARIABLE( visuals_other_remove_aim_punch, bool, false );
        CONFIG_VARIABLE( visuals_other_remove_view_punch, bool, false );
        CONFIG_VARIABLE( visuals_other_remove_post_processing, bool, false );
        CONFIG_VARIABLE( visuals_other_remove_view_bob, bool, false );
        CONFIG_VARIABLE( visuals_other_remove_scope_overlay, bool, false );
        CONFIG_VARIABLE( visuals_other_force_viewmodel_in_scope, bool, false );
        CONFIG_VARIABLE( visuals_other_remove_flash_overlay, bool, false );
        CONFIG_VARIABLE( visuals_other_remove_smoke, bool, false );

        CONFIG_VARIABLE( visuals_other_oof_arrows_color, color, color( 149, 43, 255, 150 ) );
        CONFIG_VARIABLE( visuals_other_oof_arrows, bool, false );
        CONFIG_VARIABLE( visuals_other_oof_arrows_size, int, 7 );
        CONFIG_VARIABLE( visuals_other_oof_arrows_radius, int, 91 );
        CONFIG_VARIABLE( visuals_other_oof_arrows_outline, bool, false );
        CONFIG_VARIABLE( visuals_other_oof_arrows_show_spy, bool, false );
        CONFIG_VARIABLE( visuals_other_thirdperson_enabled, bool, false );
        CONFIG_VARIABLE( visuals_other_local_blend_while_scoped, bool, false );
        CONFIG_VARIABLE( visuals_other_local_blend_transparency, float, 100.0f );
        CONFIG_VARIABLE( visuals_other_aspect_ratio, float, 100.0f );
        CONFIG_VARIABLE( visuals_other_thirdperson_interpolate, bool, false );
        CONFIG_VARIABLE( visuals_other_thirdperson_bind, int, 0 );
        CONFIG_VARIABLE( visuals_other_thirdperson_toggle, int, 0 );
        CONFIG_VARIABLE( visuals_other_thirdperson_distance, float, 120.0f );
        CONFIG_VARIABLE( visuals_other_transparent_props, bool, false );
        CONFIG_VARIABLE( visuals_other_show_server_hitboxes, bool, false );
        CONFIG_VARIABLE( visuals_other_prop_transparency, float, 100.0f );
        CONFIG_VARIABLE( visuals_other_disable_model_occlusion, bool, true );
        CONFIG_VARIABLE( visuals_render_player_chams_toggle, bool, false );
        CONFIG_VARIABLE( visuals_render_player_chams, bool, false );
        CONFIG_VARIABLE( visuals_render_player_chams_xqz, bool, false );
        CONFIG_VARIABLE( visuals_render_player_chams_show_steam_friend, bool, false );
        CONFIG_VARIABLE( visuals_render_player_chams_steam_friend_color, color, color( 0, 255, 0 ) );
        CONFIG_VARIABLE( visuals_render_player_chams_show_target, bool, false );
        CONFIG_VARIABLE( visuals_render_player_chams_lag_record, bool, false );
        CONFIG_VARIABLE( visuals_render_player_chams_lag_record_color, color, color( 238, 176, 255, 150 ) );
        CONFIG_VARIABLE( visuals_other_client_bullet_impact_col, color, color( 238, 176, 255, 150 ) );
        CONFIG_VARIABLE( visuals_other_server_bullet_impact_col, color, color( 238, 176, 255, 150 ) );
        CONFIG_VARIABLE( visuals_other_bullet_impacts, bool, false );
        CONFIG_VARIABLE( visuals_other_bullet_impacts_outline, bool, false );
       
        CONFIG_VARIABLE( visuals_render_player_chams_material, int, 0 );
#pragma endregion visual_features

#pragma region miscellaneous_features
        CONFIG_VARIABLE( misc_bypass_sv_pure, bool, true );
        CONFIG_VARIABLE( misc_modulate_console, bool, true );
        CONFIG_VARIABLE( misc_console_modulation_color, color, color( 31, 31, 31 ) );
        CONFIG_VARIABLE( misc_events_log_damage, bool, false );
        CONFIG_VARIABLE( misc_events_log_buy, bool, false );
        CONFIG_VARIABLE( misc_events_log_bomb, bool, false );
        CONFIG_VARIABLE( misc_bunny_hop, bool, false );
        CONFIG_VARIABLE( misc_fast_stop, bool, false );
        CONFIG_VARIABLE( misc_fake_walk, bool, false );
        CONFIG_VARIABLE( misc_fake_walk_key, int, 0 );
        CONFIG_VARIABLE( misc_fake_walk_key_toggle, int, 0 );
        CONFIG_VARIABLE( misc_fake_walk_value, int, 16 );
        CONFIG_VARIABLE( misc_fake_latency, bool, false );
        CONFIG_VARIABLE( misc_fake_latency_key, int, 0 );
        CONFIG_VARIABLE( misc_fake_latency_key_toggle, int, 0 );
        CONFIG_VARIABLE( misc_fake_latency_value, int, 300 );
        CONFIG_VARIABLE( misc_auto_strafe, bool, false );
        CONFIG_VARIABLE( misc_auto_strafe_type, int, strafe_type::movement_keys );
        CONFIG_VARIABLE( misc_hitmarker, bool, false );
        CONFIG_VARIABLE( misc_debug_local_overlays, int, 0 ); // local layers, queued layers, animated layers
#pragma endregion miscellaneous_features

#pragma region skinchanger_features
        CONFIG_VARIABLE( skins_override_knife, bool, false );
        CONFIG_VARIABLE( skins_override_knife_value, int, 0 );
#pragma endregion
    };
    
    enum class config_data_type : int {
        none = 0,
        data_int,
        data_bool
    };

    struct config_data {
        config_data_type t;
        bool b;
        float fl;
        int i;
    };

}// namespace config

inline std::unordered_map< int, std::unordered_map< std::string, config::config_data > > g_skin_vars = { };
inline config::variables g_vars = { };

static_assert( ( sizeof( config::variables ) % sizeof( config_bool ) ) == 0, "Config class isn't aligned properly" );
#define NUM_CONFIG_ITEMS ( sizeof( config::variables ) / sizeof( config_bool ) )