#include "is_hltv.hpp"

bool __fastcall hooks::is_hltv::hook( REGISTERS ) {
    static auto accumulate_layers_call = signature::find( XOR( "client.dll" ), XOR( "84 C0 75 0D F6 87" ) ).get< void * >( );
    
    if ( _ReturnAddress( ) == accumulate_layers_call )
        return true;

    static auto setup_velocity_call = signature::find( XOR( "client.dll" ), XOR( "84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80" ) ).get< void * >( );

    if ( _ReturnAddress( ) == setup_velocity_call )
        return true;

    return original.fastcall< bool >( REGISTERS_OUT );
}

void hooks::is_hltv::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.engine_client, 93 ),
                                          is_hltv::hook );
}