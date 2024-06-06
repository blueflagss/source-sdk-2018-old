#include "lock_cursor.hpp"
#include <features/ui/window_setup.hpp>

void __fastcall hooks::lock_cursor::hook( REGISTERS ) {
    if ( g_menu.is_opened( ) ) {
        g_interfaces.surface->unlock_cursor( );
        return;
    }

    return original.fastcall< void >( REGISTERS_OUT );
}

void hooks::lock_cursor::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.surface, 67 ),
                                          lock_cursor::hook );
}