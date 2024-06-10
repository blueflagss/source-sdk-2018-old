#pragma once

#include <globals.hpp>
#include <sdk/interfaces/base_client.hpp>
#include <sdk/interfaces/client_mode.hpp>
#include <sdk/interfaces/client_state.hpp>
#include <sdk/interfaces/collideable.hpp>
#include <sdk/other/bitbuf.h>
#include <sdk/interfaces/net_channel.hpp>
#include <sdk/interfaces/engine.hpp>
#include <sdk/interfaces/engine_trace.hpp>
#include <sdk/interfaces/entity_list.hpp>
#include <sdk/interfaces/render_view.hpp>
#include <sdk/interfaces/game_movement.hpp>
#include <sdk/interfaces/mem_alloc.hpp>
#include <sdk/interfaces/physics_surface_props.hpp>
#include <sdk/interfaces/cvar.hpp>
#include <sdk/interfaces/game_rules.hpp>
#include <sdk/interfaces/global_vars_base.hpp>
#include <sdk/interfaces/input.hpp>
#include <sdk/interfaces/input_system.hpp>
#include <sdk/interfaces/localize.hpp>
#include <sdk/interfaces/model_cache.hpp>
#include <sdk/interfaces/studio_render.hpp>
#include <sdk/interfaces/move_helper.hpp>
#include <sdk/interfaces/prediction.hpp>
#include <sdk/other/key_values.hpp>
#include <sdk/interfaces/material_system.hpp>
#include <sdk/interfaces/material.hpp>
#include <sdk/interfaces/model_render.hpp>
#include <sdk/interfaces/model_info.hpp>
#include <sdk/interfaces/surface.hpp>
#include <sdk/steam_api/isteamclient.h>
#include <sdk/steam_api/isteamutils.h>
#include <sdk/steam_api/isteamfriends.h>
#include <sdk/interfaces/debug_overlay.hpp>
#include <sdk/interfaces/panel.hpp>
#include <sdk/interfaces/event_manager.hpp>
#include <sdk/interfaces/glow_object_manager.hpp>
#include <sdk/hash/fnv1a.hpp>

class interfaces {
public:
    void init( );

    template< typename T >
    T create_interface( const char *module, const hash32_t &interface_name );

    // others.
    IDirect3DDevice9 *device = nullptr;
    HWND window_handle = nullptr;

    // interfaces.
    c_surface *surface = nullptr;
    c_panel *panel = nullptr;
    c_entity_list *entity_list = nullptr;
    c_localize *localize = nullptr;
    c_input_system *input_system = nullptr;
    c_global_vars_base *global_vars = nullptr;
    c_physics_props *physics_props = nullptr;
    c_model_info *model_info = nullptr;
    c_base_client *client = nullptr;
    c_prediction *prediction = nullptr;
    c_engine_client *engine_client = nullptr;
    c_engine_trace *engine_trace = nullptr;
    c_engine_vgui *engine_vgui = nullptr;
    c_input *input = nullptr;
    c_move_helper *move_helper = nullptr;
    c_model_cache *model_cache = nullptr;
    c_game_movement *game_movement = nullptr;
    c_client_mode *client_mode = nullptr;
    i_material_system *material_system = nullptr;
    i_model_render *model_render = nullptr;
    i_cvar *cvar = nullptr;
    i_debug_overlay *debug_overlay = nullptr;
    c_render_view *render_view = nullptr;
    ISteamClient *steam_client = nullptr;
    ISteamFriends *steam_friends = nullptr;
    ISteamUtils *steam_utils = nullptr;
    c_glow_object_mgr *glow_object_manager = nullptr;
    c_client_state *client_state = nullptr;
    c_mem_alloc *mem_alloc = nullptr;
    c_studio_render *studio_render = nullptr;
    c_game_event_mgr *event_manager = nullptr;
};

inline interfaces g_interfaces = { };

class addresses {
public:
    address md5_pseudorandom;
    address get_shotgun_spread;
    address trace_to_studio_csgo_hitgroups_priority;
    address tier0_allocated_thread_ids;
    address update_animation_state;
};

inline addresses g_addresses = { };

namespace game
{
    inline int time_to_ticks( double time ) {
        return static_cast< int >( 0.5f + static_cast< float >( time ) / g_interfaces.global_vars->interval_per_tick );
    }

    inline float ticks_to_time( int ticks ) {
        return static_cast< float >( ticks ) * g_interfaces.global_vars->interval_per_tick;
    }
}// namespace game