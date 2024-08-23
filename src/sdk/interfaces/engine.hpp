#pragma once
#include "net_channel.hpp"
#include <utils/utils.hpp>

struct player_info_t {
    std::uint64_t version;

    union {
        std::uint64_t xuid;

        struct {
            std::uint32_t xuid_low;
            std::uint32_t xuid_high;
        };
    };

    char name[ 128 ];
    int user_id;
    char steam_id[ 33 ];
    std::uint32_t friends_id;
    char friends_name[ 128 ];
    bool fake_player;
    bool is_hltv;
    std::uint32_t custom_files[ 4 ];
    std::uint8_t files_downloaded;
};

enum paint_mode : int {
    ui_panels = ( 1 << 0 ),
    in_game_panels = ( 1 << 1 ),
    cursor = ( 1 << 2 ),
};

class c_engine_vgui {
public:
    bool is_game_ui_visible( ) {
        return utils::get_method< bool( __thiscall * )( void * ) >( this, 2 )( this );
    }
};

class c_net_channel_info;

struct model_t;
struct i_material;
struct player_info;
struct key_values;

struct i_material_system;

struct c_engine_client {
public:
    void get_screen_size( int &width, int &height ) {
        utils::get_method< void( __thiscall * )( void *, int &, int & ) >( this, 5 )( this, width, height );
    }

    bool get_player_info( int index, player_info_t *info ) {
        return utils::get_method< bool( __thiscall * )( void *, int, player_info_t * ) >( this, 8 )( this, index, info );
    }

    int get_player_for_user_id( int user_id ) {
        return utils::get_method< int( __thiscall * )( void *, int ) >( this, 9 )( this, user_id );
    }

    int get_local_player( ) {
        return utils::get_method< int( __thiscall * )( void * ) >( this, 12 )( this );
    }

    float get_last_time_stamp( ) {
        return utils::get_method< float( __thiscall * )( void * ) >( this, 14 )( this );
    }

    void get_view_angles( const vector_3d &angles ) {
        return utils::get_method< void( __thiscall * )( void *, const vector_3d & ) >( this, 18 )( this, angles );
    }

    void set_view_angles( const vector_3d &angles ) {
        return utils::get_method< void( __thiscall * )( void *, const vector_3d & ) >( this, 19 )( this, angles );
    }

    int get_max_clients( ) {
        return utils::get_method< int( __thiscall * )( void * ) >( this, 20 )( this );
    }

    bool is_in_game( ) {
        return utils::get_method< bool( __thiscall * )( void * ) >( this, 26 )( this );
    }

    bool is_connected( ) {
        return utils::get_method< bool( __thiscall * )( void * ) >( this, 27 )( this );
    }

    bool is_paused( ) {
        return utils::get_method< bool( __thiscall * )( void * ) >( this, 90 )( this );
    }

    void *get_bsp_tree_query( ) {
        return utils::get_method< void *( __thiscall * ) ( void * ) >( this, 43 )( this );
    }

    c_net_channel_info *get_net_channel_info( ) {
        return utils::get_method< c_net_channel_info *( __thiscall * ) ( void * ) >( this, 78 )( this );
    }

    void execute_client_cmd( const char *cmd ) {
        utils::get_method< void( __thiscall * )( void *, const char * ) >( this, 108 )( this, cmd );
    }

    void client_cmd_unrestricted( const char *cmd ) {
        utils::get_method< void( __thiscall * )( void *, const char *, bool ) >( this, 114 )( this, cmd, false );
    }
};