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
#include <hooks/draw_set_color/draw_set_color.hpp>
#include <hooks/get_screen_aspect_ratio/get_screen_aspect_ratio.hpp>
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
#include <hooks/notify_on_layer_change_sequence/notify_on_layer_change_sequence.hpp>
#include <hooks/do_procedural_footplant/do_procedural_footplant.hpp>
#include <hooks/update_animation_state/update_animation_state.hpp>
#include <hooks/physics_simulate/physics_simulate.hpp>
#include <hooks/packet_start/packet_start.hpp>
#include <hooks/cl_fire_events/cl_fire_events.hpp>
#include <hooks/game_animation_state/game_animation_state.hpp>
#include <hooks/cl_move/cl_move.hpp>
#include <hooks/get_eye_angles/get_eye_angles.hpp>
#include <hooks/check_jump_button/check_jump_button.hpp>
#include <hooks/lower_body_yaw_target_proxy/lower_body_yaw_target_proxy.hpp>
#include <hooks/level_shutdown/level_shutdown.hpp>
#include <hooks/paint_traverse/paint_traverse.hpp>
#include <hooks/update_clientside_animations/update_clientside_animations.hpp>
#include <hooks/is_paused/is_paused.hpp>
#include <hooks/level_init_pre_entity/level_init_pre_entity.hpp>
#include <hooks/in_prediction/in_prediction.hpp>
#include <hooks/cl_dispatch_sound/cl_dispatch_sound.hpp>
#include <hooks/send_datagram/send_datagram.hpp>
#include <hooks/post_data_update/post_data_update.hpp>
#include <hooks/sequence_proxy/sequence_proxy.hpp>
#include <hooks/cl_read_packets/cl_read_packets.hpp>

std::unique_ptr< event_handler > game_event_handler = nullptr;

void hooks::impl::init( ) {
    cl_read_packets::init( );
    sequence_proxy::init( );
    post_data_update::init( );
    cl_dispatch_sound::init( );
    level_init_pre_entity::init( );
    is_paused::init( );
    //update_clientside_animations::init( );
    update_clientside_animation::init( );
    paint_traverse::init( );
    level_shutdown::init( );
    lower_body_yaw_target_proxy::init( );
    get_screen_aspect_ratio::init( );
    check_jump_button::init( );
    draw_set_color::init( );
    anim_state::init( );
    in_prediction::init( );
    get_eye_angles::init( );
    cl_move::init( );
    cl_fire_events::init( );
    packet_start::init( );
    physics_simulate::init( );
    //update_animation_state::init( );
    do_procedural_footplant::init( );
    notify_on_layer_change_cycle::init( );
    notify_on_layer_change_weight::init( );
    //notify_on_layer_change_sequence::init( );
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
    //build_transformations::init( );
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
    //// on_screen_size_changed::init( );
    frame_stage_notify::init( );

    game_event_handler = std::make_unique< event_handler >( );
}

void hooks::impl::remove( ) {
    cl_read_packets::original = { };
    post_data_update::original = { };
    send_datagram::original = { };
    cl_dispatch_sound::original = { };
    is_paused::original = { };
    get_screen_aspect_ratio::original = { };
    update_clientside_animations::original = { };
    update_clientside_animation::original = { };
    paint_traverse::original = { };
    level_shutdown::original = { };
    check_jump_button::original = { };
    setup_movement::original = { };
    setup_alive_loop::original = { };
    in_prediction::original = { };
    setup_velocity::original = { };
    get_eye_angles::original = { };
    cl_move::original = { };
    notify_on_layer_change_sequence::original = { };
    cl_fire_events::original = { };
    packet_start::original = { };
    physics_simulate::original = { };
    update_animation_state::original = { };
    do_procedural_footplant::original = { };
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
    cvar_get_bool::net_earliertempents::original = { };
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