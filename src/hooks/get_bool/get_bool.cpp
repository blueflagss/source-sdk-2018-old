#include "get_bool.hpp"

bool __fastcall hooks::cvar_get_bool::sv_cheats::hook( REGISTERS ) {
    static auto cam_think_ret_address = signature::find( "client.dll", _xs( "85 C0 75 30 38 86" ) ).get< void * >( );

    if ( _ReturnAddress( ) == cam_think_ret_address )
        return true;

    return original.fastcall< bool >( REGISTERS_OUT );
}

bool __fastcall hooks::cvar_get_bool::net_earliertempents::hook( REGISTERS ) {
    static auto cl_readpackets = signature::find( _xs( "client.dll" ), _xs( "85 C0 74 05 E8 ? ? ? ? 84 DB 0F 84" ) ).get< void * >( );

    if ( _ReturnAddress( ) == cl_readpackets )
        return true;

    return original.fastcall< bool >( REGISTERS_OUT );
}

bool __fastcall hooks::cvar_get_bool::net_showfragments::hook( REGISTERS ) {
    if ( !g_interfaces.engine_client->is_in_game( ) )
        return original.fastcall< bool >( REGISTERS_OUT );

    static auto read_sub_channel_data_ret = signature::find( _xs( "engine.dll" ), _xs( "85 C0 74 12 53 FF 75 0C 68 ? ? ? ? FF 15 ? ? ? ? 83 C4 0C" ) ).get< uintptr_t * >( );
    static auto check_receiving_list_ret = signature::find( _xs( "engine.dll" ), _xs( "8B 1D ? ? ? ? 85 C0 74 16 FF B6" ) ).get< uintptr_t * >( );

    static uint32_t last_fragment = 0;

    if ( _ReturnAddress( ) == reinterpret_cast< void * >( read_sub_channel_data_ret ) && last_fragment > 0 ) {
        const auto data = &reinterpret_cast< uint32_t * >( g_interfaces.client_state->net_channel )[ 0x56 ];
        const auto bytes_fragments = reinterpret_cast< uint32_t * >( data )[ 0x43 ];

        if ( bytes_fragments == last_fragment ) {
            auto &buffer = reinterpret_cast< uint32_t * >( data )[ 0x42 ];
            buffer = 0;
        }
    }

    if ( _ReturnAddress( ) == check_receiving_list_ret ) {
        const auto data = &reinterpret_cast< uint32_t * >( g_interfaces.client_state->net_channel )[ 0x56 ];
        const auto bytes_fragments = reinterpret_cast< uint32_t * >( data )[ 0x43 ];

        last_fragment = bytes_fragments;
    }

    return original.fastcall< bool >( REGISTERS_OUT );
}

void hooks::cvar_get_bool::init( ) {
    sv_cheats::original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.cvar->find_var( HASH_CT( "sv_cheats" ) ), 13 ),
                                                     sv_cheats::hook );

    net_showfragments::original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.cvar->find_var( HASH_CT( "net_showfragments" ) ), 13 ),
                                                             net_showfragments::hook );

    //net_earliertempents::original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.cvar->find_var( HASH_CT( "net_earliertempents" ) ), 12 ),
    //                                                           net_earliertempents::hook );
}