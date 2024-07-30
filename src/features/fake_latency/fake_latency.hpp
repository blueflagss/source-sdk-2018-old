#pragma once
#include <globals.hpp>

class fake_latency {
public:
    void on_pre_send_datagram( i_net_channel *pNetChannel, int *pnOldInReliableState, int *pnOldInSequenceNr );
    void on_post_send_datagram( i_net_channel *pNetChannel, const int nOldInReliableState, const int nOldInSequenceNr );
    void update_latency_sequences( );
    void clear_latency_sequences( );
};

inline fake_latency g_fake_latency;