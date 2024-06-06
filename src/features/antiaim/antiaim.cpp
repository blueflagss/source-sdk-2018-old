//#include "antiaim.hpp"
//
//void antiaim::on_create_move( c_user_cmd *cmd ) {
//    if ( !g_vars.exploits_antiaim.value )
//        return;
//
//    if ( !g_interfaces.engine_client->is_in_game( ) || !g_interfaces.engine_client->is_connected( ) )
//        return;
//
//    if ( !globals::local_player )
//        return;
//
//    if ( !globals::local_player->alive( ) )
//        return;
//
//    if ( cmd->buttons & buttons::use || cmd->buttons & buttons::attack )
//        return;
//
//    if ( *globals::packet && globals::old_packet )
//        *globals::packet = false;
//
//    if ( !*globals::packet ) {
//        cmd->view_angles = globals::view_angles;
//    } else {
//        *globals::packet = true;
//
//        const vector_3d fake_angle{ g_vars.exploits_antiaim_x.value, g_vars.exploits_antiaim_y.value, g_vars.exploits_antiaim_z.value };
//        cmd->view_angles = fake_angle;
//    }
//}
