#include "post_data_update.hpp"
#include <features/skin_changer/skin_changer.hpp>

void __fastcall hooks::post_data_update::hook( REGISTERS, int update_type ) {
    if ( !g_interfaces.engine_client->is_connected( ) || !g_interfaces.engine_client->is_in_game( ) )
        return original.fastcall< void >( REGISTERS_OUT, update_type );
    
    g_skin_changer.on_post_data_update_start( );

    return original.fastcall< void >( REGISTERS_OUT, update_type );
}

void hooks::post_data_update::init( ) {
    original = safetyhook::create_inline( signature::find( "client.dll", "55 8B EC 53 56 8B F1 57 80 ? ? ? ? ? ? 74 0A" ).get< void * >( ),
                                          post_data_update::hook );
}