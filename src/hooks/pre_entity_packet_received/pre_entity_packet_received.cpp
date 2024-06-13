#include "pre_entity_packet_received.hpp"
#include <features/engine_prediction/netvar_compression.hpp>
#include <features/animations/animation_sync.hpp>

void __fastcall hooks::pre_entity_packet_received::hook( REGISTERS, int commands_acknowledged, int current_world_update_packet, int server_ticks_elapsed ) {
    globals::local_player = g_interfaces.entity_list->get_client_entity< c_cs_player * >( g_interfaces.engine_client->get_local_player( ) );

    if ( globals::local_player && globals::local_player->alive( ) && commands_acknowledged > 0 ) {
        //void *pred_frame = globals::local_player->get_predicted_frame( commands_ack - 1 );
        //if ( pred_frame ) {
        //    c_predictioncopy copy_helper( PC_EVERYTHING, reinterpret_cast< void * >( globals::local_player ), TD_OFFSET_NORMAL, pred_frame, TD_OFFSET_PACKED, c_predictioncopy::TRANSFERDATA_COPYONLY );
        //    copy_helper.transfer_data( "PreEntityPacketReceived", globals::local_player->index( ), globals::local_player->get_pred_desc_map( ) );
        //}
        //memcpy( g_animations.tranny_code_premium_layers.data( ), globals::local_player->anim_overlays( ), g_animations.tranny_code_premium_layers.size( ) );
        g_netvar_compression.pre_update( globals::local_player );
    }

    return original.fastcall< void >( REGISTERS_OUT, commands_acknowledged, current_world_update_packet, server_ticks_elapsed );
}

void hooks::pre_entity_packet_received::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.prediction, 4 ),
                                          pre_entity_packet_received::hook );
}