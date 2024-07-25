#include "in_prediction.hpp"

bool __fastcall hooks::in_prediction::hook( REGISTERS ) {
    static auto setup_bones_call = signature::find( _xs( "client.dll" ), _xs( "84 C0 74 0A F3 0F 10 05 ?? ?? ?? ?? EB 05" ) ).get< void * >( );

    if ( _ReturnAddress( ) == setup_bones_call )
        return false;

    return original.fastcall< bool >( REGISTERS_OUT );
}

void hooks::in_prediction::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.prediction, 14 ),
                                          in_prediction::hook );
}