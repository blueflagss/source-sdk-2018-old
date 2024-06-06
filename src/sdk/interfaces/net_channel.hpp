#pragma once
#include "sdk/other/bitbuf.h"
#include <utils/utils.hpp>

#define FLOW_OUTGOING 0
#define FLOW_INCOMING 1
#define MAX_FLOWS 2

class i_net_channel_info {
public:
    enum {
        GENERIC = 0, // must be first and is default group
        LOCALPLAYER, // bytes for local player entity update
        OTHERPLAYERS,// bytes for other players update
        ENTITIES,    // all other entity bytes
        SOUNDS,      // game sounds
        EVENTS,      // event messages
        USERMESSAGES,// user messages
        ENTMESSAGES, // entity messages
        VOICE,       // voice data
        STRINGTABLE, // a stringtable update
        MOVE,        // client move cmds
        STRINGCMD,   // string command
        SIGNON,      // various signondata
        TOTAL,       // must be last and is not a real group
    };

    virtual const char *get_name( void ) const = 0;    // get channel name
    virtual const char *get_address( void ) const = 0; // get channel ip address as string
    virtual float get_time( void ) const = 0;          // current net time
    virtual float get_time_connected( void ) const = 0;// get connection time in seconds
    virtual int get_buffer_size( void ) const = 0;     // netchannel packet history size
    virtual int get_data_rate( void ) const = 0;       // send data rate in byte/sec
    virtual bool is_loopback( void ) const = 0;  // true if loopback channel
    virtual bool is_timing_out( void ) const = 0;// true if timing out
    virtual bool is_playback( void ) const = 0;  // true if demo playback
    virtual float get_latency( int flow ) const = 0;                                  // current latency (rtt), more accurate but jittering
    virtual float get_avg_latency( int flow ) const = 0;                              // average packet latency in seconds
    virtual float get_avg_loss( int flow ) const = 0;                                 // avg packet loss[0..1]
    virtual float get_avg_choke( int flow ) const = 0;                                // avg packet choke[0..1]
    virtual float get_avg_data( int flow ) const = 0;                                 // data flow in bytes/sec
    virtual float get_avg_packets( int flow ) const = 0;                              // avg packets/sec
    virtual int get_total_data( int flow ) const = 0;                                 // total flow in/out in bytes
    virtual int get_sequence_nr( int flow ) const = 0;                                // last send seq number
    virtual bool is_valid_packet( int flow, int frame_number ) const = 0;             // true if packet was not lost/dropped/chocked/flushed
    virtual float get_packet_time( int flow, int frame_number ) const = 0;            // time when packet was send
    virtual int get_packet_bytes( int flow, int frame_number, int group ) const = 0;  // group size of this packet
    virtual bool get_stream_progress( int flow, int *received, int *total ) const = 0;// tcp progress if transmitting
    virtual float get_time_since_last_received( void ) const = 0;                     // get time since last recieved packet in seconds
    virtual float get_command_interpolation_amount( int flow, int frame_number ) const = 0;
    virtual void get_packet_response_latency( int flow, int frame_number, int *pn_latency_msecs, int *pn_choke ) const = 0;
    virtual void get_remote_framerate( float *pfl_frame_time, float *pfl_frame_time_std_deviation ) const = 0;

    virtual float get_timeout_seconds( ) const = 0;
};

class i_demo_recorder;
class i_net_channel_handler;

typedef struct netpacket_s netpacket_t;
typedef struct netadr_s netadr_t;

class i_net_channel : public i_net_channel_info {
public:
    virtual ~i_net_channel( void ){ };

    virtual void set_data_rate( float rate ) = 0;
    virtual bool register_message( void *msg ) = 0;
    virtual bool start_streaming( unsigned int challenge_nr ) = 0;
    virtual void reset_streaming( void ) = 0;
    virtual void set_timeout( float seconds ) = 0;
    virtual void set_demo_recorder( i_demo_recorder *recorder ) = 0;
    virtual void set_challenge_nr( unsigned int chnr ) = 0;
    virtual void reset( void ) = 0;
    virtual void clear( void ) = 0;
    virtual void shutdown( const char *reason ) = 0;
    virtual void process_playback( void ) = 0;
    virtual bool process_stream( void ) = 0;
    virtual void process_packet( struct netpacket_s *packet, bool b_has_header ) = 0;
    virtual bool send_net_msg( void *msg, bool b_force_reliable = false, bool b_voice = false ) = 0;
    virtual bool send_data( void *msg, bool b_reliable = true ) = 0;
    virtual bool send_file( const char *filename, unsigned int transfer_id ) = 0;
    virtual void deny_file( const char *filename, unsigned int transfer_id ) = 0;
    virtual void request_file_old( const char *filename, unsigned int transfer_id ) = 0;// get rid of this function when we version the
    virtual void set_choked( void ) = 0;
    virtual int send_datagram( void *data ) = 0;
    virtual bool transmit( bool only_reliable = false ) = 0;
    virtual const netadr_t &get_remote_address( void ) const = 0;
    virtual i_net_channel_handler *get_msg_handler( void ) const = 0;
    virtual int get_drop_number( void ) const = 0;
    virtual int get_socket( void ) const = 0;
    virtual unsigned int get_challenge_nr( void ) const = 0;
    virtual void get_sequence_data( int &n_out_sequence_nr, int &n_in_sequence_nr, int &n_out_sequence_nr_ack ) = 0;
    virtual void set_sequence_data( int n_out_sequence_nr, int n_in_sequence_nr, int n_out_sequence_nr_ack ) = 0;
    virtual void update_message_stats( int msggroup, int bits ) = 0;
    virtual bool can_packet( void ) const = 0;
    virtual bool is_overflowed( void ) const = 0;
    virtual bool is_timed_out( void ) const = 0;
    virtual bool has_pending_reliable_data( void ) = 0;
    virtual void set_file_transmission_mode( bool b_background_mode ) = 0;
    virtual void set_compression_mode( bool b_use_compression ) = 0;
    virtual unsigned int request_file( const char *filename ) = 0;
    virtual float get_time_since_last_received( void ) const = 0;// get time since last received packet in seconds
    virtual void set_max_buffer_size( bool b_reliable, int n_bytes, bool b_voice = false ) = 0;
    virtual bool is_null( ) const = 0;
    virtual int get_num_bits_written( bool b_reliable ) = 0;
    virtual void set_interpolation_amount( float fl_interpolation_amount ) = 0;
    virtual void set_remote_framerate( float fl_frame_time, float fl_frame_time_std_deviation ) = 0;

    // max # of payload bytes before we must split/fragment the packet
    virtual void set_max_routable_payload_size( int n_split_size ) = 0;
    virtual int get_max_routable_payload_size( ) = 0;

    virtual int get_protocol_version( ) = 0;

    bool processing_messages;
    bool should_delete;
    int out_sequence_nr;
    int in_sequence_nr;
    int out_sequence_nr_ack;
    int out_reliable_state;
    int in_reliable_state;
    int choked_packets;
    int packet_drop;
};

class i_net_message {
public:
    virtual ~i_net_message( ){ };

    // use these to setup who can hear whose voice.
    // pass in client indices (which are their ent indices - 1).

    virtual void set_net_channel( i_net_channel *netchan ) = 0;// netchannel this message is from/for
    virtual void set_reliable( bool state ) = 0;               // set to true if it's a reliable message
    virtual bool process( void ) = 0;// calles the recently set handler to process this message
    virtual bool read_from_buffer( void *buffer ) = 0;// returns true if parsing was ok
    virtual bool write_to_buffer( void *buffer ) = 0; // returns true if writing was ok
    virtual bool is_reliable( void ) const = 0;// true, if message needs reliable handling
    virtual int get_type( void ) const = 0;        // returns module specific header tag eg svc_serverinfo
    virtual int get_group( void ) const = 0;       // returns net message group of this message
    virtual const char *get_name( void ) const = 0;// returns network message name, eg "svc_serverinfo"
    virtual i_net_channel *get_net_channel( void ) const = 0;
    virtual const char *to_string( void ) const = 0;// returns a human readable string about message content
};

class c_net_message : public i_net_message {
public:
    c_net_message( ) {
        reliable = true;
        net_channel = nullptr;
    }

    virtual ~c_net_message( ){ };

    virtual int get_group( ) const { return i_net_channel_info::GENERIC; }
    i_net_channel *get_net_channel( ) const { return net_channel; }

    virtual void set_reliable( bool state ) { reliable = state; };
    virtual bool is_reliable( ) const { return reliable; };
    virtual void set_net_channel( i_net_channel *netchan ) { net_channel = netchan; }

    virtual bool process( ) {
        assert( 0 );
        return false;
    };

protected:
    bool reliable;
    i_net_channel *net_channel;
};

class clc_voicedata : public c_net_message {
public:
    int length;
    bf_read data_in;
    bf_write data_out;
    uint64_t xuid;
};