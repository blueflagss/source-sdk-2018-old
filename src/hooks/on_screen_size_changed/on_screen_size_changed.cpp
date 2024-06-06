#include "on_screen_size_changed.hpp"

void __fastcall hooks::on_screen_size_changed::hook( REGISTERS, int old_wide, int old_tall ) {
    int width, height;
    g_interfaces.engine_client->get_screen_size( width, height );

    globals::screen_size = {
            static_cast< float >( width ),
            static_cast< float >( height )
    };

    return original.fastcall< void >( REGISTERS_OUT, old_wide, old_tall );
}

void hooks::on_screen_size_changed::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.surface, 111 ),
                                          on_screen_size_changed::hook );
}