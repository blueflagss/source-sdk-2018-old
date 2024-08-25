#pragma once
#include <precompiled.hpp>
#include <sdk/math/matrix3x4.hpp>
#include <sdk/interfaces/cvar.hpp>
#include <sdk/interfaces/model_render.hpp>
#include <sdk/interfaces/global_vars_base.hpp>
#include <features/renderer/renderer.hpp>
#include <core/addresses.hpp>
#include <sdk/user_cmd.hpp>

class c_cs_weapon_base;
class c_cs_player;
class c_cs_weapon_info;

struct network_data_t {
    int sequence;
    int cmd;
};

namespace globals
{
    inline std::array< const char *, 14 > sky_names = {
            _xs( "cs_baggage_skybox_" ),
            _xs( "cs_tibet" ),
            _xs( "vietnam" ),
            _xs( "sky_lunacy" ),
            _xs( "embassy" ),
            _xs( "italy" ),
            _xs( "jungle" ),
            _xs( "office" ),
            _xs( "sky_cs15_daylight01_hdr" ),
            _xs( "sky_cs15_daylight02_hdr" ),
            _xs( "sky_day02_05" ),
            _xs( "sky_csgo_cloudy01" ),
            _xs( "sky_csgo_night02" ),
            _xs( "sky_csgo_night02b" ),
    };

    namespace ui
    {
        inline float frametime{ 0.0f };
        inline vector_2d screen_size{ 0.0f, 0.0f };
        inline vector_2d old_screen_size{ 0.0f, 0.0f };
    }// namespace ui

    namespace hotkeys
    {
        inline bool thirdperson{ false };
        inline bool fake_latency{ false };
        inline bool manual_left{ false };
        inline bool manual_right{ false };
        inline bool manual_back{ false };
    }// namespace hotkeys

    namespace cvars
    {
        inline convar *sv_accelerate{ nullptr };
        inline convar *molotov_throw_detonate_time{ nullptr };
        inline convar *weapon_molotov_maxdetonateslope{ nullptr };
        inline convar *cl_extrapolate{ nullptr };
        inline convar *sv_friction{ nullptr };
        inline convar *sv_stopspeed{ nullptr };
        inline convar *cl_interp_ratio{ nullptr };
        inline convar *sv_client_min_interp_ratio{ nullptr };
        inline convar *cl_updaterate{ nullptr };
        inline convar *cl_sidespeed{ nullptr };
        inline convar *cl_forwardspeed{ nullptr };
        inline convar *cl_upspeed{ nullptr };
        inline convar *sv_maxunlag{ nullptr };
        inline convar *sv_client_max_interp_ratio{ nullptr };
        inline convar *cl_predict{ nullptr };
        inline convar *cl_lagcompensation{ nullptr };
        inline convar *sv_maxupdaterate{ nullptr };
        inline convar *cam_ideallag{ nullptr };
        inline convar *sv_minupdaterate{ nullptr };
        inline convar *r_DrawSpecificStaticProp{ nullptr };
        inline convar *cl_foot_contact_shadows{ nullptr };
        inline convar *cl_csm_shadows{ nullptr };
        inline convar *name{ nullptr };
        inline convar *sv_maxusrcmdprocessticks{ nullptr };
        inline convar *cl_cmdrate{ nullptr };
        inline convar *sv_gravity{ nullptr };
        inline convar *sv_jump_impulse{ nullptr };
        inline convar *weapon_accuracy_shotgun_spread_patterns{ nullptr };
        inline convar *weapon_recoil_scale{ nullptr };
        inline convar *cl_interp{ nullptr };
        inline convar *mp_damage_scale_ct_body{ nullptr };
        inline convar *mp_damage_scale_t_body{ nullptr };
        inline convar *mp_damage_scale_ct_head{ nullptr };
        inline convar *mp_damage_scale_t_head{ nullptr };
        inline convar *sv_penetration_type{ nullptr };
        inline convar *ff_damage_reduction_bullets{ nullptr };
        inline convar *ff_damage_bullet_penetration{ nullptr };
        inline convar *sv_clip_penetration_traces_to_players{ nullptr };
        inline convar *mat_fullbright{ nullptr };

        void init( );
    }// namespace cvars

    inline bool is_targetting{ false };
    inline bool did_setup_datamap{ false };
    inline int target_index{ -1 };
    inline int tick_rate{ -1 };
    inline c_user_cmd *user_cmd{ nullptr };
    inline c_user_cmd sent_user_cmd{ };
    inline int arrival_tick{ };
    inline int server_tick{ };
    inline int latency_ticks{ };
    inline int last_reliable{ };
    inline float latency{ };
    inline vector_3d view_angles{ };
    inline vector_3d sent_angles{ };
    inline vector_3d angles{ };
    inline vector_3d local_shoot_pos{ };
    inline v_matrix view_matrix{ };
    inline bool lby_updating{ false };
    inline bool is_console_toggled{ false };
    inline bool *packet{ nullptr };
    inline bool old_packet{ false };
    inline vector_2d screen_size{ };
    inline c_cs_player *local_player{ nullptr };
    inline vector_3d local_angles{ };
    inline c_cs_weapon_base *local_weapon{ nullptr };
    inline c_cs_weapon_info *local_weapon_data{ nullptr };
    inline std::array< bool, 64 > allow_animations{ };
    inline bool allow_bones = false;
    inline std::array< int, 3 > backup_clientstate_vars = { };
    inline c_global_vars_base backup_global_vars = { };
    inline float lerp_amount{ 0.0f };
    inline std::array< network_data_t, 150 > outgoing_cmds{ };
    inline std::deque< int > sent_commands{ };
}// namespace globals