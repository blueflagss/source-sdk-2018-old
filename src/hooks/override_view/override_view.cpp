#include "override_view.hpp"

void __fastcall hooks::override_view::hook( REGISTERS, c_view_setup *setup ) {
    if ( !globals::local_player || !g_interfaces.engine_client->is_in_game( ) || !g_interfaces.engine_client->is_connected( ) )
        return original.fastcall< void >( REGISTERS_OUT, setup );

    /* the size of the bounding hull used for collision checking. */
    constexpr float cam_hull_offset{ 16.0f };

    if ( !globals::local_player->alive( ) ) {
        g_interfaces.input->cam_to_thirdperson( );
        g_interfaces.input->camera_in_thirdperson = true;
        globals::hotkeys::thirdperson = true;

        return original.fastcall< void >( REGISTERS_OUT, setup );
    }

    if ( g_vars.visuals_other_thirdperson_enabled.value && globals::hotkeys::thirdperson ) {
        g_interfaces.input->cam_to_thirdperson( );

        const vector_3d cam_hull_min( -cam_hull_offset, -cam_hull_offset, -cam_hull_offset );
        const vector_3d cam_hull_max( cam_hull_offset, cam_hull_offset, cam_hull_offset );

        vector_3d angles;
        g_interfaces.engine_client->get_view_angles( angles );

        vector_3d fwd, origin = globals::shoot_position;

        math::angle_vectors( angles, &fwd );
        angles.z = g_vars.visuals_other_thirdperson_distance.value;

        c_trace_filter_hitscan filter;
        filter.skip = globals::local_player;

        c_game_trace tr;
        ray_t ray;
        ray.init( origin, origin - ( fwd * angles.z ), cam_hull_min, cam_hull_max );

        g_interfaces.engine_trace->trace_ray( ray, mask_npcworldstatic, &filter, &tr );
        angles.z *= tr.fraction;

        g_interfaces.input->camera_offset = angles;
        g_interfaces.input->camera_in_thirdperson = true;

        globals::hotkeys::thirdperson = true;
    } else {
        g_interfaces.input->cam_to_firstperson( );
        g_interfaces.input->camera_in_thirdperson = false;

        globals::hotkeys::thirdperson = false;
    }

    if ( g_vars.visuals_other_fov.value > 0 )
        setup->fov = g_vars.visuals_other_fov.value;

    return original.fastcall< void >( REGISTERS_OUT, setup );
}

void hooks::override_view::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.client_mode, 18 ),
                                          override_view::hook );
}