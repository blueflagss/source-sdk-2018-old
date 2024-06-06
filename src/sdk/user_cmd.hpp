#pragma once

#include "other/crc32.h"

enum buttons : int {
    attack = ( 1 << 0 ),
    jump = ( 1 << 1 ),
    duck = ( 1 << 2 ),
    forward = ( 1 << 3 ),
    back = ( 1 << 4 ),
    use = ( 1 << 5 ),
    cancel = ( 1 << 6 ),
    left = ( 1 << 7 ),
    right = ( 1 << 8 ),
    move_left = ( 1 << 9 ),
    move_right = ( 1 << 10 ),
    attack2 = ( 1 << 11 ),
    run = ( 1 << 12 ),
    reload = ( 1 << 13 ),
    alt1 = ( 1 << 14 ),
    alt2 = ( 1 << 15 ),
    score = ( 1 << 16 ),
    speed = ( 1 << 17 ),
    walk = ( 1 << 18 ),
    zoom = ( 1 << 19 ),
    weapon1 = ( 1 << 20 ),
    weapon2 = ( 1 << 21 ),
    bullrush = ( 1 << 22 ),
};

class c_user_cmd {
public:
    PAD( 0x4 );

    inline uint32_t get_checksum( void ) const {
        uint32_t crc;

        crc_32::init( &crc );
        crc_32::process_buffer( &crc, &command_number, sizeof( command_number ) );
        crc_32::process_buffer( &crc, &tick_count, sizeof( tick_count ) );
        crc_32::process_buffer( &crc, &view_angles, sizeof( view_angles ) );
        crc_32::process_buffer( &crc, &aim_direction, sizeof( aim_direction ) );
        crc_32::process_buffer( &crc, &forward_move, sizeof( forward_move ) );
        crc_32::process_buffer( &crc, &side_move, sizeof( side_move ) );
        crc_32::process_buffer( &crc, &up_move, sizeof( up_move ) );
        crc_32::process_buffer( &crc, &buttons, sizeof( buttons ) );
        crc_32::process_buffer( &crc, &impulse, sizeof( impulse ) );
        crc_32::process_buffer( &crc, &weapon_select, sizeof( weapon_select ) );
        crc_32::process_buffer( &crc, &weapon_subtype, sizeof( weapon_subtype ) );
        crc_32::process_buffer( &crc, &random_seed, sizeof( random_seed ) );
        crc_32::process_buffer( &crc, &mousedx, sizeof( mousedx ) );
        crc_32::process_buffer( &crc, &mousedy, sizeof( mousedy ) );
        crc_32::final( &crc );

        return crc;
    }

    int command_number;
    int tick_count;
    vector_3d view_angles;
    vector_3d aim_direction;
    float forward_move;
    float side_move;
    float up_move;
    int buttons;
    unsigned char impulse;
    int weapon_select;
    int weapon_subtype;
    int random_seed;
    short mousedx;
    short mousedy;
    bool predicted;
    PAD( 24 );
};

class c_verified_user_cmd {
public:
    c_user_cmd cmd;
    uint32_t crc_hash;
};