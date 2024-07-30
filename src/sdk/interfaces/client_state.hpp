#pragma once

#define MD5_DIGEST_LENGTH 16
#define MD5_BIT_LENGTH ( MD5_DIGEST_LENGTH * sizeof( unsigned char ) )

#include "engine.hpp"
#include "net_channel.hpp"
#include "sdk/other/utlvector.hpp"
#include "utils/netvars/netvars.hpp"
#include "utils/padding.hpp"

enum {
    NUCLOCKDRIFT_SAMPLES = 16
};

class c_clock_drift_mgr {
public:
    float clock_offsets[ NUCLOCKDRIFT_SAMPLES ];
    int current_offset = 0;
    int server_tick = 0;
    int client_tick = 0;
};

#define MAX_AREA_STATE_BYTES 32
#define MAX_AREA_PORTAL_STATE_BYTES 24

struct add_angle {
    float total;
    float starttime;
};

typedef unsigned int crc32_t;
typedef struct custofile {
    crc32_t crc;       //file CRC
    unsigned int reqID;// download request ID
} CustomFile_t;

class c_client_class;

class c_event_info {
public:
    enum {
        EVENT_INDEX_BITS = 8,
        EVENT_DATA_LEN_BITS = 11,
        MAX_EVENT_DATA = 192,// ( 1<<8 bits == 256, but only using 192 below )
    };

    ~c_event_info( ) {
        if ( pData ) {
            delete pData;
        }
    }

    // 0 implies not in use
    short class_id;

    // If non-zero, the delay time when the event should be fired ( fixed up on the client )
    float fire_delay;

    // send table pointer or NULL if send as full update
    const void *pSendTable;
    const c_client_class *client_class;

    // Length of data bits
    int bits;
    // Raw event data
    char *pData;
    // CLIENT ONLY Reliable or not, etc.
    int flags;

private:
    char pad[ 24 ];

public:
    c_event_info *next;
};

class c_client_class;

class c_net_channel_info {
public:

    float get_latency( int flow ) {
        return utils::get_method< float( __thiscall * )( void *, int ) >( this, 9 )( this, flow );
    }

    float get_avg_latency( int flow ) {
        return utils::get_method< float( __thiscall * )( void *, int ) >( this, 10 )( this, flow );
    }
};

class c_net_channel {
public:
    PAD( 0x3F );
    int out_sequence_nr;
    int in_sequence_nr;
    int out_sequence_nr_ack;
    int out_reliable_state;
    int in_reliable_state;
    int choked_packets;

    bool send_net_msg( void *msg, bool reliable, bool voice ) {
        return utils::get_method< bool( __thiscall * )( void *, void *, bool, bool ) >( this, 40 )( this, msg, reliable, voice );
    }

    void send_datagram( ) {
        utils::get_method< int( __thiscall * )( void *, void * ) >( this, 48 )( this, 0 );
    }
};

class client_event_t {
public:
    enum {
        event_index_bits = 8,
        event_data_len_bits = 11,
        max_event_data = 192,
    };

    short class_id;
    float fire_delay;
    const void *send_table;
    const c_client_class *client_class;
    int bits;
    uint8_t *data;
    int flags;
    PAD( 0x18 );
    client_event_t *next;
};

class c_client_state {
public:
    PAD( 156 );
    c_net_channel *net_channel;

    OFFSET( server_tick, int, 0x016C );
    OFFSET( last_command_ack, int, 0x4CB4 );
    OFFSET( delta_tick, int, 0x174 );
    OFFSET( last_outgoing_command, int, 0x4CAC );
    OFFSET( events, client_event_t *, 0x4DEC );
    OFFSET( choked_commands, int, 0x4CB0 );
};