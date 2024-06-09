#include "frame_stage_notify.hpp"
#include <features/animations/animation_sync.hpp>
#include <features/engine_prediction/engine_prediction.hpp>
#include <features/visuals/visuals.hpp>

void __fastcall hooks::frame_stage_notify::hook( REGISTERS, client_frame_stage stage ) {
    if ( !globals::local_player )
        return original.fastcall< void >( REGISTERS_OUT, stage );

    static auto override_post_processing_disable = signature::find( XOR( "client.dll" ), XOR( "80 3D ? ? ? ? ? 53 56 57 0F 85" ) ).add( 0x2 ).deref( ).get< bool * >( );

    auto backup_view_punch = globals::local_player->view_punch( );
    auto backup_aim_punch = globals::local_player->aim_punch( );

    if ( stage == frame_render_start ) {
        if ( override_post_processing_disable )
            *override_post_processing_disable = g_vars.visuals_other_remove_post_processing.value;

        g_visuals.world_modulation( );

        if ( g_vars.visuals_other_remove_flash_overlay.value ) {
            if ( globals::local_player )
                globals::local_player->flash_duration( ) = 0.0f;
        }

        if ( g_vars.visuals_other_remove_aim_punch.value )
            globals::local_player->aim_punch( ) = vector_3d( 0.f, 0.f, 0.f );

        if ( g_vars.visuals_other_remove_view_punch.value )
            globals::local_player->view_punch( ) = vector_3d( 0.f, 0.f, 0.f );

        g_animations.on_pre_frame_render_start( );
    }

    if ( stage == frame_net_update_end ) {
        //const auto view_model = g_interfaces.entity_list->get_client_entity_from_handle< c_view_model * >( globals::local_player->view_model( ) );

        //if ( view_model && globals::local_player->view_model( ) != 0xFFFFFFF ) {
        //    /* restore viewmodel when model renders a scene */
        //    //view_model->sequence( ) = g_prediction.sequence;
        //    //view_model->animation_parity( ) = g_prediction.animation_parity;
        //}
       
        g_animations.maintain_local_animations( );
    }

    original.fastcall< void >( REGISTERS_OUT, stage );

    if ( stage == frame_render_start ) {
        g_animations.on_post_frame_render_start( );

        if ( g_vars.visuals_other_show_server_hitboxes.value ) {
            float duration = -1.f;

            static auto draw_server_hitboxes = signature::find( XOR( "server.dll" ), XOR( "E8 ? ? ? ? F6 83 ? ? ? ? ? 0F 84 ? ? ? ? 33 FF 39 BB" ) ).add( 0x1 ).rel32( ).get< void * >( );
            static auto util_player_by_index = signature::find( XOR( "server.dll" ), XOR( "85 C9 7E 2A A1" ) ).get< c_cs_player *( __fastcall * ) ( int ) >( );

            for ( int i = 1; i < g_interfaces.global_vars->max_clients; i++ ) {
                auto e = g_interfaces.entity_list->get_client_entity< c_cs_player * >( i );

                if ( !e )
                    continue;

                auto ent = util_player_by_index( e->index( ) );

                if ( !ent )
                    continue;

                __asm {
					    pushad
					    movss xmm1, duration
					    push 1//bool monoColor
					    mov ecx, ent
					    call draw_server_hitboxes
					    popad
                }
            }
        }

        if ( !g_vars.visuals_other_remove_aim_punch.value )
            globals::local_player->aim_punch( ) = backup_aim_punch;

        if ( !g_vars.visuals_other_remove_view_punch.value )
            globals::local_player->view_punch( ) = backup_view_punch;
    }

    /* run lag-compensation when we get last received data. */
    if ( stage == frame_net_update_end ) 
        g_animations.on_net_update_end( );
}

void hooks::frame_stage_notify::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.client, 36 ),
                                          frame_stage_notify::hook );
}