#include "end_scene.hpp"
#include <features/features.hpp>

long __fastcall hooks::end_scene::hook( REGISTERS, IDirect3DDevice9 *device ) {
    static auto return_address = _ReturnAddress( );

    if ( return_address != _ReturnAddress( ) )
        return original.fastcall< long >( REGISTERS_OUT, device );

    static bool initialized = false;

    if ( !initialized ) {
        auto hwnd = FindWindowA( "Valve001", nullptr );

        if ( hwnd )
            render::init( hwnd, device );

        globals::ui::old_screen_size = globals::ui::screen_size;

        initialized = true;

        return original.fastcall< long >( REGISTERS_OUT, device );
    }

    render::begin_draw_states( device );
    {
        penumbra::input::update( "Valve001" );

        render::begin( [ & ] {
            g_visuals.render( );
            g_notify.render( );
            g_menu.render( );
        } );

        render::show_objects( );
    }

    render::finish_draw_states( device );

    return original.fastcall< long >( REGISTERS_OUT, device );
}

void hooks::end_scene::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.device, 42 ),
                                          end_scene::hook );
}