#include "frame_stage_notify.hpp"
#include <features/animations/animation_sync.hpp>
#include <features/engine_prediction/engine_prediction.hpp>
#include <features/visuals/visuals.hpp>

void __fastcall hooks::frame_stage_notify::hook( REGISTERS, client_frame_stage stage ) {
    if ( !globals::local_player )
        return original.fastcall< void >( REGISTERS_OUT, stage );

    static auto override_post_processing_disable = signature::find( XOR( "client.dll" ), XOR( "80 3D ? ? ? ? ? 53 56 57 0F 85" ) ).add( 0x2 ).deref( ).get< bool * >( );

    if ( override_post_processing_disable )
        *override_post_processing_disable = g_vars.visuals_other_remove_post_processing.value;

    const auto backup_aim_punch = globals::local_player->aim_punch( );
    const auto backup_view_punch = globals::local_player->view_punch( );

    g_animations.on_pre_frame_stage_notify( stage );

    if ( stage == frame_render_start ) {
        g_visuals.world_modulation( );

        if ( g_vars.visuals_other_remove_flash_overlay.value ) {
            if ( globals::local_player )
                globals::local_player->flash_duration( ) = 0.0f;
        }

        if ( g_vars.visuals_other_remove_aim_punch.value )
            globals::local_player->aim_punch( ) = vector_3d( 0.f, 0.f, 0.f );

        if ( g_vars.visuals_other_remove_view_punch.value )
            globals::local_player->view_punch( ) = vector_3d( 0.f, 0.f, 0.f );
    }

    if ( stage == frame_net_update_end ) {
        if ( globals::local_player && globals::local_player->alive( ) ) {
            const auto view_model = g_interfaces.entity_list->get_client_entity_from_handle< c_view_model * >( globals::local_player->viewmodel_handle( ) );

            /* restore viewmodel when model renders a scene */
            if ( view_model && globals::local_player->viewmodel_handle( ) != 0xFFFFFFF ) {
                view_model->cycle( ) = g_prediction.weapon_cycle;
                view_model->sequence( ) = g_prediction.weapon_sequence;
                view_model->animtime( ) = g_prediction.weapon_animtime;
            }

            g_animations.maintain_local_animations( );
        }
    }

    original.fastcall< void >( REGISTERS_OUT, stage );

    g_animations.on_post_frame_stage_notify( stage );

    if ( stage == frame_render_start ) {
        if ( !g_vars.visuals_other_remove_aim_punch.value )
            globals::local_player->aim_punch( ) = backup_aim_punch;

        if ( !g_vars.visuals_other_remove_view_punch.value )
            globals::local_player->view_punch( ) = backup_view_punch;
    }

}

void hooks::frame_stage_notify::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.client, 36 ),
                                          frame_stage_notify::hook );
}