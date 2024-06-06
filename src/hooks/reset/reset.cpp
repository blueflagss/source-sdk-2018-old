#include "reset.hpp"
#include <features/ui/window_setup.hpp>

long __stdcall hooks::reset::hook( IDirect3DDevice9 *device, D3DPRESENT_PARAMETERS *presentation_parameters ) {
    auto hr = original.stdcall< long >( device, presentation_parameters );

    ImGui_ImplDX9_InvalidateDeviceObjects( );

    if ( SUCCEEDED( hr ) ) {
        ImGui_ImplDX9_CreateDeviceObjects( );
        
        globals::ui::screen_size = {
                presentation_parameters->BackBufferWidth,
                presentation_parameters->BackBufferHeight 
        };

        g_menu.on_screen_size_updated( presentation_parameters->BackBufferWidth, presentation_parameters->BackBufferHeight );
    }

    return hr;
}

void hooks::reset::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.device, 16 ),
                                          reset::hook );
}