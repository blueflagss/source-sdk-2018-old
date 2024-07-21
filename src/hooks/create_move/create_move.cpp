#include "create_move.hpp"
#include <features/prediction_manager/prediction_manager.hpp>
#include <features/ragebot/ragebot.hpp>
#include <features/movement/movement.hpp>
#include <features/sound_handler/sound_handler.hpp>
#include <features/antiaim/antiaim.hpp>
#include <features/visuals/visuals.hpp>

float calculate_lerp( ) {
    auto updaterate = std::clamp< float >( globals::cvars::cl_updaterate->get_float( ), globals::cvars::sv_minupdaterate->get_float( ), globals::cvars::sv_maxupdaterate->get_float( ) );
    auto lerp_ratio = std::clamp< float >( globals::cvars::cl_interp_ratio->get_float( ), globals::cvars::sv_client_min_interp_ratio->get_float( ), globals::cvars::sv_client_max_interp_ratio->get_float( ) );

    return std::clamp< float >( lerp_ratio / updaterate, globals::cvars::cl_interp->get_float( ), 1.0f );
}

void apply_leg_movement( c_user_cmd *cmd ) { /* premium */
    float side_move;                          // xmm2_4
    int new_buttons;                         // eax
    float forward_move;                       // xmm1_4

    side_move = cmd->side_move;
    new_buttons = cmd->buttons & ~0x618u;
    forward_move = cmd->forward_move;

    if ( !g_vars.exploits_antiaim_leg_movement.value ) {
        if ( forward_move <= 0.0 ) {
            if ( forward_move < 0.0 )
                new_buttons |= buttons::back;
        } else {
            new_buttons |= buttons::forward;
        }

        if ( side_move > 0.0 )
            goto LABEL_15;

        if ( side_move >= 0.0 )
            goto LABEL_18;
        goto LABEL_17;
    }

    if ( g_vars.exploits_antiaim_leg_movement.value != 1 )
        goto LABEL_18;

    if ( forward_move <= 0.0 ) {
        if ( forward_move < 0.0 )
            new_buttons |= buttons::forward;
    } else {
        new_buttons |= buttons::back;
    }
    if ( side_move > 0.0 ) {
    LABEL_17:
        new_buttons |= buttons::move_left;
        goto LABEL_18;
    }
    if ( side_move < 0.0 )
    LABEL_15:
        new_buttons |= buttons::move_right;
LABEL_18:
    cmd->buttons = new_buttons;
}

void backup_players( bool restore ) {
    // restore stuff.
    for ( int i{ 1 }; i <= g_interfaces.global_vars->max_clients; ++i ) {
        c_cs_player *player = g_interfaces.entity_list->get_client_entity< c_cs_player * >( i );

        if ( !player || player->dormant( ) || !player->alive( ) || player == globals::local_player || player->team( ) == globals::local_player->team( ) )
            continue;

        if ( restore )
            g_ragebot.m_backup[ i - 1 ].restore( player );
        else
            g_ragebot.m_backup[ i - 1 ].store( player );
    }
}

bool __fastcall hooks::create_move::hook( REGISTERS, float input_sample_time, c_user_cmd *cmd ) {
    const auto ret = original.fastcall< bool >( REGISTERS_OUT, input_sample_time, cmd );

    if ( !cmd || !cmd->command_number )
        return ret;

    if ( !globals::local_player )
        return ret;

    if ( ret )
        g_interfaces.engine_client->set_view_angles( cmd->view_angles );

    std::uintptr_t *stack_ptr;
    __asm mov stack_ptr, ebp

    globals::user_cmd = cmd;
    globals::packet = reinterpret_cast< bool * >( *reinterpret_cast< uintptr_t * >( stack_ptr ) - 0x1C );

    const auto net_channel = g_interfaces.engine_client->get_net_channel_info( );

    /* Generate random seed since CInput::CreateMove isn't called so we must do it ourselves. */
    cmd->random_seed = g_addresses.md5_pseudorandom.get< unsigned int( __cdecl * )( unsigned int ) >( )( cmd->command_number ) & 0x7FFFFFFF;

    if ( net_channel != nullptr ) {
        globals::latency = net_channel->get_latency( 0 );
        globals::latency_ticks = game::time_to_ticks( globals::latency );
        globals::lerp_amount = calculate_lerp( );
        globals::server_tick = g_interfaces.client_state->server_tick( );
        globals::arrival_tick = globals::server_tick + globals::latency_ticks;
    }

    const auto old_cmd_angles = cmd->view_angles;

    backup_players( false );

    if ( g_interfaces.client_state->delta_tick( ) < 0 ) {
        g_interfaces.prediction->update(
                g_interfaces.client_state->delta_tick( ),
                g_interfaces.client_state->delta_tick( ) > 0,
                g_interfaces.client_state->last_command_ack( ),
                g_interfaces.client_state->last_outgoing_command( ) + g_interfaces.client_state->choked_commands( )
        );
    }

    if ( globals::local_player->alive( ) ) {
        if ( globals::local_player ) {
            globals::local_weapon = g_interfaces.entity_list->get_client_entity_from_handle< c_cs_weapon_base * >( globals::local_player->weapon_handle( ) );

            if ( globals::local_weapon )
                globals::local_weapon_data = globals::local_weapon->get_weapon_data( );
        }

        g_interfaces.engine_client->get_view_angles( globals::view_angles );

        if ( g_vars.exploits_fakelag.value ) {
            if ( math::length_2d( g_prediction_context.velocity ) > 5.0f )
                *globals::packet = g_interfaces.client_state->choked_commands( ) >= g_vars.exploits_fakelag_limit.value;
        }

        g_movement.on_create_move( cmd, globals::view_angles );
        g_antiaim.on_create_move( cmd, cmd->view_angles );

        g_prediction_context.start( cmd );
        {
            g_visuals.on_create_move( );
            g_sound_handler.on_create_move( );
            g_ragebot.on_create_move( cmd );

            if ( !*globals::packet )
                globals::sent_user_cmd = *cmd;
        }
        g_prediction_context.finish( cmd );

        vector_3d engine_angles;

        g_interfaces.engine_client->get_view_angles( engine_angles );
        g_interfaces.engine_client->set_view_angles( math::clamp_angle( engine_angles ) );
        g_movement.correct_movement( cmd, math::clamp_angle( globals::view_angles ) );

        apply_leg_movement( cmd );
        g_animations.update_local_animations( cmd );
    } else {
        globals::local_weapon = nullptr;
    }

    backup_players( true );   
    
	//if ( !*globals::packet ) { 
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