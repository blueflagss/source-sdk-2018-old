#include "fake_latency.hpp"

struct sequence_data_t {
    int in_reliable_state = 0;
    int in_sequence_nr = 0;
    float real_time = 0.f;
};

static std::deque< sequence_data_t > sequence_cache = { };
static int last_incoming_sequence = 0;

void fake_latency::on_pre_send_datagram( i_net_channel *net_channel, int *old_in_reliable_state, int *old_in_sequence_nr ) {
    if ( !g_vars.misc_fake_latency.value && !globals::hotkeys::fake_latency )
        return;

    *old_in_reliable_state = net_channel->in_reliable_state;
    *old_in_sequence_nr = net_channel->in_sequence_nr;

    const float desired_latency = ( g_vars.misc_fake_latency_value.value / 1000.0f ) - globals::latency;

    for ( const auto &sequence : sequence_cache ) {
        if ( ( g_interfaces.global_vars->realtime - sequence.real_time ) >= desired_latency ) {
            net_channel->in_reliable_state = sequence.in_reliable_state;
            net_channel->in_sequence_nr = sequence.in_sequence_nr;
            break;
        }
    }
}

void fake_latency::on_post_send_datagram( i_net_channel *net_channel, const int old_in_reliable_state, const int old_in_sequence_nr ) {
    if ( !g_vars.misc_fake_latency.value && !globals::hotkeys::fake_latency )
        return;

    net_channel->in_reliable_state = old_in_reliable_state;
    net_channel->in_sequence_nr = old_in_sequence_nr;
}

void fake_latency::update_latency_sequences( ) {
    const auto net_channel = reinterpret_cast< i_net_channel * >( g_interfaces.engine_client->get_net_channel_info( ) );

    if ( !net_channel )
        return;

    if ( last_incoming_sequence == 0 ) {
        last_incoming_sequence = net_channel->in_sequence_nr;
        return;
    }

    if ( net_channel->in_sequence_nr > last_incoming_sequence ) {
        last_incoming_sequence = net_channel->in_sequence_nr;
        sequence_cache.emplace_front( sequence_data_t{ net_channel->in_reliable_state, net_channel->in_sequence_nr, g_interfaces.global_vars->realtime } );
    }

    if ( sequence_cache.size( ) > 1024U )
        sequence_cache.pop_back( );
}

void fake_latency::clear_latency_sequences( ) {
    last_incoming_sequence = 0;

    if ( !sequence_cache.empty( ) )
        sequence_cache.clear( );
}