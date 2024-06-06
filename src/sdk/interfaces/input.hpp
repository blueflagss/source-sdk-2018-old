#pragma once
#include <utils/utils.hpp>
#include <utils/padding.hpp>
#include <sdk/user_cmd.hpp>

class c_input {
public:
    void cam_to_thirdperson( ) {
        return utils::get_method< void( __thiscall * )( void * ) >( this, 35 )( this );
    }

    void cam_to_firstperson( ) {
        return utils::get_method< void( __thiscall * )( void * ) >( this, 36 )( this );
    }

    void *vftable;
    PAD( 160 );
    bool camera_intercepting_mouse;
    bool camera_in_thirdperson;
    PAD( 2 );
    vector_3d camera_offset;
    PAD( 56 );
    c_user_cmd *cmds;
    c_verified_user_cmd *verified_cmds;
    PAD( 84 );

    inline c_user_cmd *get_commands( int sequence_number ) {
        return &cmds[ sequence_number % 150 ];
    }

    inline c_verified_user_cmd *get_verified_commands( int sequence_number ) {
        return &verified_cmds[ sequence_number % 150 ];
    }
};