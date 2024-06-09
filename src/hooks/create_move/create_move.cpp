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

void setup_shoot_position( ) {
    if ( !globals::local_player )
        return;

    globals::shoot_position = globals::local_player->origin( ) + globals::local_player->view_offset( );

    if ( globals::local_player->alive( ) ) {
        auto backup_pose = globals::local_player->pose_parameters( )[ 12 ];

        globals::local_player->pose_parameters( )[ 12 ] = ( math::normalize_angle( globals::local_player->aim_punch( ).x * globals::cvars::weapon_recoil_scale->get_float( ), -180.0f, 180.0f ) + 90.f ) / 180.f;

        std::array< matrix_3x4, 128 > bones;
        const auto ret = g_animations.build_bones( globals::local_player, bones.data( ), g_interfaces.global_vars->curtime );

        globals::local_player->pose_parameters( )[ 12 ] = backup_pose;

        const auto state = globals::local_player->anim_state( );

        if ( ret && state )
            globals::local_player->modify_eye_position( state, &globals::shoot_position, bones.data( ) );
    }
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
    static auto MD5_PseudoRandom = signature::find( "client.dll", "E8 ? ? ? ? 25 ? ? ? ? B9" ).add( 0x1 ).rel32( ).get< unsigned int( __cdecl * )( unsigned int ) >( );

    const auto ret = original.fastcall< bool >( REGISTERS_OUT, input_sample_time, cmd );

    //if ( cmd && ret )
    //    g_interfaces.engine_client->set_view_angles( cmd->view_angles );

    if ( !cmd || !cmd->command_number )
        return ret;

    if ( !globals::local_player )
        return ret;

    globals::user_cmd = cmd;
    globals::packet = reinterpret_cast< bool * >( *reinterpret_cast< std::uintptr_t * >( reinterpret_cast< std::uintptr_t >( _AddressOfReturnAddress( ) ) - sizeof( std::uintptr_t * ) ) - 0x1C );

    const auto net_channel = g_interfaces.engine_client->get_net_channel_info( );

    setup_shoot_position( );

    // Generate random seed since CInput::CreateMove isn't called so we must do it ourselves.
    cmd->random_seed = MD5_PseudoRandom( cmd->command_number ) & 0x7FFFFFFF;

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

        vector_3d old_angles = math::clamp_angle( globals::view_angles );

        vector_3d engine_angles;
        g_interfaces.engine_client->get_view_angles( engine_angles );

        auto angle = math::clamp_angle( engine_angles );

        g_interfaces.engine_client->set_view_angles( angle );
        g_movement.correct_movement( cmd, old_angles );
    }

    backup_players( true );

    globals::old_packet = *globals::packet;
  
    return false;
}

void hooks::create_move::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.client_mode, 24 ),
                                          create_move::hook );
}