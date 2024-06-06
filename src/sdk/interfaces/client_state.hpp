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

class client_class_t;

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
    PAD( 18 );
    int out_sequence_nr;
    int isequence_nr;
    int out_sequence_nr_ack;
    int out_reliable_state;
    int in_reliable_state;
    int choked_packets;

    bool send_net_msg( void *msg, bool reliable, bool voice ) {
        return utils::get_method< bool( __thiscall * )( void *, void *, bool, bool ) >( this, 40 )( this, msg, reliable, voice );
    }

    void send_datagram( ) {
        utils::get_method< int( __thiscall * )( void *, void * ) >( this, 46 )( this, 0 );
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
    const client_class_t *client_class;
    int bits;
    uint8_t *data;
    int flags;
    PAD( 0x1C );
    client_event_t *next;
};

class c_client_state {
public:
    PAD( 156 );
    c_net_channel *net_channel;
    int challenge_nr;
    PAD( 4 );
    double connect_time;
    int retry_number;
    PAD( 84 );
    int signon_state;
    PAD( 4 );
    double next_cmd_time;
    int server_count;
    int current_sequence;
    PAD( 8 );

    struct {
        float clock_offsets[ 16 ];
        int cur_clock_offset;
        int server_tick;
        int client_tick;
    } clock_drift_mgr;

    int delta_tick;
    PAD( 4 );
    int view_entity;
    int player_slot;
    bool paused;
    PAD( 3 );
    char level_name[ 260 ];
    char level_name_short[ 40 ];
    PAD( 212 );
    int max_clients;
    PAD( 18836 );
    int old_tickcount;
    float tick_remainder;
    float frametime;
    int last_outgoing_command;
    int choked_commands;
    int last_command_ack;
    int last_server_tick;
    int command_ack;
    int sound_sequence;
    int last_progress_percent;
    bool is_hltv;
    PAD( 75 );
    vector_3d ang;
    PAD( 204 );
    client_event_t *events;
};