#pragma once
#include <precompiled.hpp>
#include <sdk/math/matrix3x4.hpp>
#include <sdk/interfaces/cvar.hpp>
#include <sdk/interfaces/model_render.hpp>
#include <features/renderer/renderer.hpp>

class c_cs_weapon_base;
class c_user_cmd;
class c_cs_player;

namespace globals
{
    inline std::array< const char *, 14 > sky_names = {
            XOR( "cs_baggage_skybox_" ),
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
    }

    namespace cvars
    {
        inline convar *cl_extrapolate{ nullptr };
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
        inline convar *sv_minupdaterate{ nullptr };
        inline convar *r_DrawSpecificStaticProp{ nullptr };
        inline convar *cl_foot_contact_shadows{ nullptr };
        inline convar *cl_csm_shadows{ nullptr };
        inline convar *name{ nullptr };
        inline convar *sv_maxusrcmdprocessticks{ nullptr };
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

        void init( );
    }// namespace cvars

    inline bool is_targetting{ false };
    inline bool did_setup_datamap{ false };
    inline int target_index{ -1 };
    inline c_user_cmd *user_cmd{ nullptr };
    inline vector_3d view_angles{ };
    inline v_matrix view_matrix{ };
    inline bool *packet{ nullptr };
    inline vector_2d screen_size{ };
    inline vector_3d shoot_position{ };
    inline c_cs_player *local_player{ nullptr };
    inline c_cs_weapon_base *local_weapon{ nullptr };
    inline std::array< bool, 64 > allow_animations{ };
    inline std::array< bool, 65 > is_building_bones{ };
    inline float lerp_amount{ 0.0f };
    inline std::deque< int > sent_commands{ };
}// namespace globals