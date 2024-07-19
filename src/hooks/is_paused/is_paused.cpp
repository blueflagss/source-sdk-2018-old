#include "is_paused.hpp"

bool __fastcall hooks::is_paused::hook( REGISTERS ) {
    static auto interp_server_entities_call = signature::find( _xs( "client.dll" ), _xs( "84 C0 74 07 C6 05 ? ? ? ? ? 8B" ) ).get< void * >( );

    //if ( _ReturnAddress( ) == interp_server_entities_call )
    //    return true;

    return original.fastcall< bool >( REGISTERS_OUT );
}

void hooks::is_paused::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.engine_client, 90 ),
                                          is_paused::hook );
}