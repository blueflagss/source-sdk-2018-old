#include "sound_handler.hpp"

void sound_handler::on_create_move( ) {
    if ( !g_interfaces.engine_client->is_connected( ) || !g_interfaces.engine_client->is_in_game( ) )
        return;

    if ( !globals::local_player )
        return;

    c_utl_vector< sndinfo_t > active_sounds { };

    active_sounds.RemoveAll( );
    g_interfaces.engine_sound->get_active_sounds( active_sounds );

    if ( !active_sounds.m_Size ) return;

    for ( int i = 0; i < active_sounds.m_Size; i++ ) {
        const auto sound = &active_sounds[ i ];

        if ( !sound || sound->m_nSoundSource == globals::local_player->index( ) || sound->m_nSoundSource < 0 || sound->m_nSoundSource > 64 )
            continue;

        auto player = g_interfaces.entity_list->get_client_entity< c_cs_player * >( sound->m_nSoundSource );

        if ( !player )
            continue;

        if ( !player->alive( ) || !player->is_player( ) || player->team( ) == globals::local_player->team( ) ) 
            continue;

        if ( !player->dormant( ) ) {
            dormant_players[ player->index( ) ].last_update_time = -1;
            dormant_players[ player->index( ) ].origin = player->get_render_origin( );
            dormant_players[ player->index( ) ].visible = false;
            continue;
        }

        set_dormant_origin( sound );
        dormant_players[ sound->m_nSoundSource ].player = player;
    }
}

void sound_handler::reset_data( ) {
    for ( int i = 1; i < 64; i++ )
        dormant_players[ i ].reset( );
}

void sound_handler::on_round_start( event_t *evt ) {
    if ( !evt )
        return;

    reset_data( );
}

void sound_handler::update_position( c_cs_player *player ) {
    const auto &dormant_info = g_sound_handler.dormant_players[ player->index( ) ];

    if ( dormant_info.last_update_time == -1 ) {
        player->set_abs_origin( player->get_render_origin( ) );
        player->origin( ) = player->get_render_origin( );
        player->rgfl_coordinate_frame( ).set_origin( player->get_render_origin( ) );
        return;
    }

    if ( !player->dormant( ) )
        return;

    if ( dormant_info.origin != player->origin( ) || g_interfaces.global_vars->realtime != dormant_info.last_update_time ) {
        player->set_abs_origin( dormant_info.origin );
        player->origin( ) = dormant_info.origin;
        player->rgfl_coordinate_frame( ).set_origin( dormant_info.origin );
    }
}

void sound_handler::set_dormant_origin( sndinfo_t *sound ) {
    if ( sound->m_pOrigin == nullptr ) return;

    ray_t ray;

    auto src_origin = ( *sound->m_pOrigin ) + vector_3d( 0, 0, 1 );
    auto dst_origin = src_origin - vector_3d( 0, 0, 100 );

    ray.init( src_origin, dst_origin );

    c_game_trace tr;
    c_trace_filter_hitscan filter;
    filter.player = globals::local_player;

    g_interfaces.engine_trace->trace_ray( ray, mask_playersolid, &filter, &tr );

    dormant_players[ sound->m_nSoundSource ].visible = true;
    dormant_players[ sound->m_nSoundSource ].last_update_time = g_interfaces.global_vars->realtime;
    dormant_players[ sound->m_nSoundSource ].origin = ( tr.fraction < 0.97 ) ? tr.end_pos : *sound->m_pOrigin;
}