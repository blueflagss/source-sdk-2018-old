#include "create_move.hpp"
#include <core/config.hpp>
#include <core/variables.hpp>
#include <features/features.hpp>
#include <features/penetration/penetration.hpp>

float calculate_lerp( ) {
    auto updaterate = std::clamp< float >( globals::cvars::cl_updaterate->get_float( ), globals::cvars::sv_minupdaterate->get_float( ), globals::cvars::sv_maxupdaterate->get_float( ) );
    auto lerp_ratio = std::clamp< float >( globals::cvars::cl_interp_ratio->get_float( ), globals::cvars::sv_client_min_interp_ratio->get_float( ), globals::cvars::sv_client_max_interp_ratio->get_float( ) );

    return std::clamp< float >( lerp_ratio / updaterate, globals::cvars::cl_interp->get_float( ), 1.0f );
}

void apply_leg_movement( c_user_cmd *cmd ) { /* premium */
    float sidemove;                          // xmm2_4
    int new_buttons;                         // eax
    float forwardmove;                       // xmm1_4

    sidemove = cmd->side_move;
    new_buttons = cmd->buttons & ~0x618u;
    forwardmove = cmd->forward_move;

    if ( !g_vars.exploits_antiaim_leg_movement.value ) {
        if ( forwardmove <= 0.0 ) {
            if ( forwardmove < 0.0 )
                new_buttons |= buttons::back;
        } else {
            new_buttons |= buttons::forward;
        }

        if ( sidemove > 0.0 )
            goto LABEL_15;

        if ( sidemove >= 0.0 )
            goto LABEL_18;
        goto LABEL_17;
    }

    if ( g_vars.exploits_antiaim_leg_movement.value != 1 )
        goto LABEL_18;

    if ( forwardmove <= 0.0 ) {
        if ( forwardmove < 0.0 )
            new_buttons |= buttons::forward;
    } else {
        new_buttons |= buttons::back;
    }
    if ( sidemove > 0.0 ) {
    LABEL_17:
        new_buttons |= buttons::move_left;
        goto LABEL_18;
    }
    if ( sidemove < 0.0 )
    LABEL_15:
        new_buttons |= buttons::move_right;
LABEL_18:
    cmd->buttons = new_buttons;
}

void backup_players( bool restore ) {
    // restore stuff.
    for ( int i{ 1 }; i <= g_interfaces.global_vars->max_clients; ++i ) {
        c_cs_player *player = g_interfaces.entity_list->get_client_entity< c_cs_player * >( i );

        if ( !player || !player->alive( ) || player == globals::local_player || player->team( ) == globals::local_player->team( ) )
            continue;

        if ( restore )
            g_aimbot.m_backup[ i - 1 ].restore( player );
        else
            g_aimbot.m_backup[ i - 1 ].store( player );
    }
}

bool __fastcall hooks::create_move::hook( REGISTERS, float input_sample_time, c_user_cmd *cmd ) {
    const auto ret = original.fastcall< bool >( REGISTERS_OUT, input_sample_time, cmd );

    if ( !cmd || !cmd->command_number )
        return ret;

    if ( !globals::local_player )
        return ret;

    globals::user_cmd = cmd;
    globals::packet = reinterpret_cast< bool * >( *reinterpret_cast< std::uintptr_t * >( reinterpret_cast< std::uintptr_t >( _AddressOfReturnAddress( ) ) - sizeof( std::uintptr_t * ) ) - 0x1C );

    const auto net_channel = g_interfaces.engine_client->get_net_channel_info( );

    /* Generate random seed since CInput::CreateMove isn't called so we must do it ourselves. */
    cmd->random_seed = g_addresses.md5_pseudorandom.get< unsigned int( __cdecl * )( unsigned int ) >( )( cmd->command_number ) & 0x7FFFFFFF;

    globals::lerp_amount = calculate_lerp( );

    const auto old_cmd_angles = cmd->view_angles;

    backup_players( false );

    if ( globals::local_player->alive( ) ) {
        if ( globals::local_player )
            globals::local_weapon = g_interfaces.entity_list->get_client_entity_from_handle< c_cs_weapon_base * >( globals::local_player->weapon_handle( ) );

        g_interfaces.engine_client->get_view_angles( globals::view_angles );
        g_movement.on_create_move( cmd, globals::view_angles );
       
        g_prediction.start( cmd );
        {
            if ( math::length_2d( g_prediction.predicted_velocity ) > 5.0f )
                *globals::packet = g_interfaces.client_state->choked_commands( ) >= g_vars.exploits_fakelag_limit.value;

            g_antiaim.on_create_move( cmd, cmd->view_angles );
            g_aimbot.on_create_move( cmd );
        }
        g_prediction.finish( cmd );

        vector_3d engine_angles;

        g_interfaces.engine_client->get_view_angles( engine_angles );
        g_interfaces.engine_client->set_view_angles( math::clamp_angle( engine_angles ) );
        g_movement.correct_movement( cmd, math::clamp_angle( globals::view_angles ) );

        if ( *globals::packet )
            globals::sent_angles = cmd->view_angles;

        apply_leg_movement( cmd );
    }

    backup_players( true );
    
	//if ( *globals::packet ) {
 //       globals::sent_commands.emplace_front( cmd->command_number );
 //   } 
 //   
 //   else {
 //       auto net_channel = g_interfaces.client_state->net_channel;

 //       if ( net_channel ) {
 //           const int backup_choked = net_channel->choked_packets;

 //           net_channel->choked_packets = 0;
 //           net_channel->send_datagram( );
 //           //net_channel->out_sequence_nr--;
 //           --*reinterpret_cast< int * >( reinterpret_cast< uintptr_t >( net_channel ) + 0x18 );
 //           //--*reinterpret_cast< int * >( reinterpret_cast< uintptr_t >( net_channel ) + 0x2C );
 //           net_channel->choked_packets = backup_choked;
 //       }
 //   }

    globals::old_packet = *globals::packet;
  
    return false;
}

void hooks::create_move::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.client_mode, 24 ),
                                          create_move::hook );
}