#include "get_bool.hpp"

bool __fastcall hooks::cvar_get_bool::sv_cheats::hook( REGISTERS ) {
    static auto cam_think_ret_address = signature::find( "client.dll", XOR( "85 C0 75 30 38 86" ) ).get< void * >( );

    if ( _ReturnAddress( ) == cam_think_ret_address )
        return true;

    return original.fastcall< bool >( REGISTERS_OUT );
}

bool __fastcall hooks::cvar_get_bool::net_earliertempents::hook( REGISTERS ) {
    static auto cl_readpackets = signature::find( "client.dll", XOR( "85 C0 74 05 E8 ? ? ? ? 84 DB 0F 84" ) ).get< void * >( );

    if ( _ReturnAddress( ) == cl_readpackets )
        return true;

    return original.fastcall< bool >( REGISTERS_OUT );
}

void hooks::cvar_get_bool::init( ) {
    sv_cheats::original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.cvar->find_var( HASH_CT( "sv_cheats" ) ), 13 ),
                                                     sv_cheats::hook );

    net_earliertempents::original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.cvar->find_var( HASH_CT( "net_earliertempents" ) ), 12 ),
                                                               net_earliertempents::hook );
}