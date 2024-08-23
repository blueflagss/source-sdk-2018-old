#include "send_datagram.hpp"
#include <features/fake_latency/fake_latency.hpp>

int __fastcall hooks::send_datagram::hook( REGISTERS, void *datagram ) {
    if ( datagram  != nullptr || !g_interfaces.engine_client->is_connected( ) || !g_interfaces.engine_client->is_in_game( ) )
        return original.fastcall< int >( REGISTERS_OUT, datagram );

        if ( !g_interfaces.engine_client->is_in_game( ) )
        return original.fastcall< int >( REGISTERS_OUT, datagram );

        const auto net_channel = reinterpret_cast< i_net_channel * >( ecx );

    const int backup_seq = net_channel->in_sequence_nr;
        const auto reliable_changed = net_channel->in_reliable_state == globals::last_reliable;
    globals::last_reliable = net_channel->in_reliable_state;

    const int seq_spike = game::time_to_ticks( g_vars.misc_fake_latency_value.value * 0.001f - globals::latency );
    if ( g_vars.misc_fake_latency.value && seq_spike > 0 && reliable_changed && net_channel->in_sequence_nr > seq_spike ) {
        net_channel->in_sequence_nr -= seq_spike;
    }

    const int retval = original.fastcall< int >( REGISTERS_OUT, datagram );
    net_channel->in_sequence_nr = backup_seq;

    return retval;
}

void hooks::send_datagram::init( ) {
    //original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.engine_client->get_net_channel_info( ), 48 ),
    //                                      send_datagram::hook );
}
