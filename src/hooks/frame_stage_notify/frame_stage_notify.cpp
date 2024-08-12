#include "frame_stage_notify.hpp"
#include <features/features.hpp>
#include <features/prediction_manager/prediction_manager.hpp>
#include <features/skin_changer/skin_changer.hpp>

struct client_hit_verify_t {
    vector_3d pos;
    float time;
    float duration;
};

std::vector< client_hit_verify_t * > client_hit;

void __fastcall hooks::frame_stage_notify::hook( REGISTERS, client_frame_stage stage ) {
    globals::local_player = ( !g_interfaces.engine_client->is_in_game( ) || !g_interfaces.engine_client->is_connected( ) ) ? nullptr : g_interfaces.entity_list->get_client_entity< c_cs_player * >( g_interfaces.engine_client->get_local_player( ) );

    if ( !globals::local_player )
        return original.fastcall< void >( REGISTERS_OUT, stage );

    static auto override_post_processing_disable = signature::find( _xs( "client.dll" ), _xs( "80 3D ? ? ? ? ? 53 56 57 0F 85" ) ).add( 0x2 ).deref( ).get< bool * >( );
    static auto m_vecBulletVerifyListClient = signature::find( _xs( "client.dll" ), _xs( "8B 86 ? ? ? ? F3 0F 10 17" ) ).add( 2 ).deref( ).get< ptrdiff_t >( );

    if ( override_post_processing_disable )
        *override_post_processing_disable = g_vars.visuals_other_remove_post_processing.value;

    const auto backup_aim_punch = globals::local_player->aim_punch( );
    const auto backup_view_punch = globals::local_player->view_punch( );

    g_animations.on_pre_frame_stage_notify( stage );

    if ( stage == frame_render_start && globals::local_player ) {
        g_shot_manager.process_shots( );
        g_visuals.world_modulation( );

        if ( g_vars.visuals_other_remove_flash_overlay.value )
            globals::local_player->flash_duration( ) = 0.0f;

        if ( g_vars.visuals_other_remove_aim_punch.value )
            globals::local_player->aim_punch( ) = vector_3d( 0.f, 0.f, 0.f );

        if ( g_vars.visuals_other_remove_view_punch.value )
            globals::local_player->view_punch( ) = vector_3d( 0.f, 0.f, 0.f );

        /* bullet impacts */
        if ( g_vars.visuals_other_bullet_impacts.value ) {
            static int last_num_impacts = 0;
            static int last_server_num_impacts = 0;

            client_hit_verify_t *last_hit = nullptr;

            int last_timestamp = 0;
            const auto num_impacts = *reinterpret_cast< c_utl_vector< client_hit_verify_t > * >( reinterpret_cast< uintptr_t >( globals::local_player ) + m_vecBulletVerifyListClient );

            if ( num_impacts.Count( ) > last_num_impacts ) {
                for ( auto i = num_impacts.Count( ) - 1; i >= 0; i-- ) {
                    const auto impact = &num_impacts.m_pElements[ i ];

                    if ( !impact )
                        continue;

                    if ( g_interfaces.global_vars->curtime >= last_timestamp ) {
                        last_timestamp = g_interfaces.global_vars->curtime;
                        last_hit = impact;
                        break;
                    }
                }

                client_hit.clear( );

                if ( last_hit )
                    g_interfaces.debug_overlay->add_box_overlay( last_hit->pos, vector_3d( -2.0f, -2.0f, -2.0f ), vector_3d( 2.0f, 2.0f, 2.0f ), vector_3d( 0.0f, 0.0f, 0.0f ), g_vars.visuals_other_client_bullet_impact_col.value.r, g_vars.visuals_other_client_bullet_impact_col.value.g, g_vars.visuals_other_client_bullet_impact_col.value.b, g_vars.visuals_other_client_bullet_impact_col.value.a, 7.0f );
            }

            last_num_impacts = num_impacts.Count( );
        }
    }

    if ( stage == frame_net_update_end && globals::local_player->alive( ) ) {
        const auto view_model = globals::local_player->get_view_model( );

        if ( view_model && globals::local_player->viewmodel_handle( ) != 0xFFFFFFF ) {
            view_model->cycle( ) = g_prediction_context.weapon_cycle;
            view_model->animtime( ) = g_prediction_context.weapon_animtime;
            view_model->model_index( ) = g_prediction_context.weapon_model_index;
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