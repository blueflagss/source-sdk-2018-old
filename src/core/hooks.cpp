#include "hooks.hpp"

#include <features/event_handler/event_handler.hpp>
#include <hooks/create_move/create_move.hpp>
#include <hooks/draw_model_execute/draw_model_execute.hpp>
#include <hooks/end_scene/end_scene.hpp>
#include <hooks/frame_stage_notify/frame_stage_notify.hpp>
#include <hooks/on_screen_size_changed/on_screen_size_changed.hpp>
#include <hooks/reset/reset.hpp>
#include <hooks/lock_cursor/lock_cursor.hpp>
#include <hooks/wnd_proc/wnd_proc.hpp>
#include <hooks/override_view/override_view.hpp>
#include <hooks/get_alpha_modulation/get_alpha_modulation.hpp>
#include <hooks/is_using_static_props_debug_modes/is_using_static_props_debug_modes.hpp>
#include <hooks/run_command/run_command.hpp>
#include <hooks/get_bool/get_bool.hpp>
#include <hooks/get_viewmodel_fov/get_viewmodel_fov.hpp>
#include <hooks/calc_viewmodel_view/calc_viewmodel_view.hpp>
#include <hooks/list_leaves_in_box/list_leaves_in_box.hpp>
#include <hooks/process_interpolated_list/process_interpolated_list.hpp>
#include <hooks/process_movement/process_movement.hpp>
#include <hooks/should_skip_animation_frame/should_skip_animation_frame.hpp>
#include <hooks/base_interpolate_part1/base_interpolate_part1.hpp>
#include <hooks/check_for_sequence_change/check_for_sequence_change.hpp>
#include <hooks/modify_eye_position/modify_eye_position.hpp>
#include <hooks/build_transformations/build_transformations.hpp>
#include <hooks/calc_view/calc_view.hpp>
#include <hooks/estimate_abs_velocity/estimate_abs_velocity.hpp>
#include <hooks/is_hltv/is_hltv.hpp>
#include <hooks/setup_bones/setup_bones.hpp>
#include <hooks/do_post_screen_space_effects/do_post_screen_space_effects.hpp>
#include <hooks/standard_blending_rules/standard_blending_rules.hpp>
#include <hooks/do_extra_bone_processing/do_extra_bone_processing.hpp>
#include <hooks/post_network_data_received/post_network_data_received.hpp>
#include <hooks/pre_entity_packet_received/pre_entity_packet_received.hpp>
#include <hooks/calc_viewmodel_bob/calc_viewmodel_bob.hpp>
#include <hooks/render_smoke_overlay/render_smoke_overlay.hpp>
#include <hooks/hud_scope_paint/hud_scope_paint.hpp>
#include <hooks/notify_on_layer_change_cycle/notify_on_layer_change_cycle.hpp>
#include <hooks/notify_on_layer_change_weight/notify_on_layer_change_weight.hpp>

std::unique_ptr< event_handler > game_event_handler = nullptr;

void hooks::impl::init( ) {
    notify_on_layer_change_cycle::init( );
    notify_on_layer_change_weight::init( );
    hud_scope_paint::init( );
    render_smoke_overlay::init( );
    calc_viewmodel_bob::init( );
    pre_entity_packet_received::init( );
    post_network_data_received::init( );
    do_post_screen_space_effects::init( );
    standard_blending_rules::init( );
    setup_bones::init( );
    estimate_abs_velocity::init( );
    is_hltv::init( );
    calc_view::init( );
    build_transformations::init( );
    modify_eye_position::init( );
    check_for_sequence_change::init( );
    base_interpolate_part1::init( );
    should_skip_animation_frame::init( );
    process_interpolated_list::init( );
    process_movement::init( );
    list_leaves_in_box::init( );
    calc_viewmodel_view::init( );
    get_viewmodel_fov::init( );
    cvar_get_bool::init( );
    run_command::init( );
    draw_model_execute::init( );
    is_using_static_props_debug_modes::init( );
    get_alpha_modulation::init( );
    override_view::init( );
    wnd_proc::init( );
    lock_cursor::init( );
    create_move::init( );
    end_scene::init( );
    reset::init( );
    // on_screen_size_changed::init( );
    frame_stage_notify::init( );

    game_event_handler = std::make_unique< event_handler >( );
}

void hooks::impl::remove( ) {
    hud_scope_paint::original = { };
    notify_on_layer_change_cycle::original = { };
    notify_on_layer_change_weight::original = { };
    render_smoke_overlay::original = { };
    calc_viewmodel_bob::original = { };
    pre_entity_packet_received::original = { };
    post_network_data_received::original = { };
    do_post_screen_space_effects::original = { };
    do_extra_bone_processing::original = { };
    standard_blending_rules::original = { };
    setup_bones::original = { };
    estimate_abs_velocity::original = { };
    is_hltv::original = { };
    calc_view::original = { };
    build_transformations::original = { };
    modify_eye_position::original = { };
    base_interpolate_part1::original = { };
    check_for_sequence_change::original = { };
    should_skip_animation_frame::original = { };
    process_interpolated_list::original = { };
    process_movement::original = { };
    list_leaves_in_box::original = { };
    calc_viewmodel_view::original = { };
    get_viewmodel_fov::original = { };
    cvar_get_bool::sv_cheats::original = { };
    run_command::original = { };
    is_using_static_props_debug_modes::original = { };
    get_alpha_modulation::original = { };
    override_view::original = { };
    draw_model_execute::original = { };
    lock_cursor::original = { };
    create_move::original = { };
    end_scene::original = { };
    reset::original = { };
    // on_screen_size_changed::original = { };
    wnd_proc::original = { };
    frame_stage_notify::original = { };

    SetWindowLongPtr( g_interfaces.window_handle, GWL_WNDPROC, reinterpret_cast< LONG_PTR >( wnd_proc::original ) );
    wnd_proc::original = nullptr;

    g_interfaces.engine_client->client_cmd_unrestricted( "cl_mouseenable 1" );
}