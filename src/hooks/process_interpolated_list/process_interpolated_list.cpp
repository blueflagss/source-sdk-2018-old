#include "process_interpolated_list.hpp"

void __cdecl hooks::process_interpolated_list::hook( ) {
    static auto allow_extrapolation = signature::find( _xs( "client.dll" ), _xs( "80 3D ? ? ? ? ? 8D 57" ) ).add( 0x2 ).deref( ).get< bool * >( );
    *allow_extrapolation = false;
    return original.call< void >( );
}

void hooks::process_interpolated_list::init( ) {
    original = safetyhook::create_inline( signature::find( _xs( "client.dll" ), _xs( "0F ? ? ? ? ? ? 3D ? ? ? ? 74 3F" ) ).get< void * >( ),
                                          process_interpolated_list::hook );
}