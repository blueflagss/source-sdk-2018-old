#pragma once
#include <sdk/datamap.hpp>
#include <utils/utils.hpp>

class c_base_player;
class c_user_cmd;

typedef void ( *fn_field_compare_t )( const char *classname, const char *fieldname, const char *fieldtype, bool networked, bool noterrorchecked,
                                      bool differs, bool withintolerance, const char *value );

class c_prediction {
public:
    void update( int start_frame, bool valid_frame, int incoming_acknowledged, int outgoing_cmd ) {
        return utils::get_method< void( __thiscall * )( void *, int, bool, int, int ) >( this, 3 )( this, start_frame, valid_frame, incoming_acknowledged, outgoing_cmd );
    }

    void check_moving_ground( c_cs_player *player, double frametime ) {
        return utils::get_method< void( __thiscall * )( void *, c_cs_player *, double ) >( this, 18 )( this, player, frametime );
    }

    void set_local_view_angles( vector_3d &ang ) {
        return utils::get_method< void( __thiscall * )( void *, vector_3d & ) >( this, 13 )( this, ang );
    }

    void setup_move( c_cs_player *player, c_user_cmd *ucmd, void *move_helper, c_move_data *move ) {
        return utils::get_method< void( __thiscall * )( void *, c_cs_player *, c_user_cmd *, void *, c_move_data * ) >( this, 20 )( this, player, ucmd, move_helper, move );
    }

    void finish_move( c_cs_player *player, c_user_cmd *ucmd, c_move_data *move ) {
        return utils::get_method< void( __thiscall * )( void *, c_cs_player *, c_user_cmd *, c_move_data * ) >( this, 21 )( this, player, ucmd, move );
    }

    PAD( 4 );
    std::uintptr_t last_ground;
    bool in_prediction;
    bool first_time_predicted;
    bool engine_paused;
    bool old_pred;
    int prev_start_frame;
    int incoming_packet_num;
    PAD( 8 );
    int cmds_predicted;
    int server_commands_ack;
    int prev_ack_had_errors;
    float ideal_pitch;
    int last_command_ack;
};