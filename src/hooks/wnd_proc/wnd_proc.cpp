#include "wnd_proc.hpp"
#include <features/ui/window_setup.hpp>

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

LRESULT __stdcall hooks::wnd_proc::hook( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam ) {
    penumbra::input::wnd_proc( hwnd, msg, wparam, lparam );

    bool allow_input_processing = false;

    if ( g_menu.is_opened( ) ) {
        allow_input_processing = true;

        switch ( msg ) {
            case WM_KEYDOWN: {
                if ( LOWORD( wparam ) == VK_SPACE || LOWORD( wparam ) == 0x57 || LOWORD( wparam ) == 0x41 || LOWORD( wparam ) == 0x53 || LOWORD( wparam ) == 0x44 )
                    allow_input_processing = false;
            } break;
            case WM_KEYUP: {
                if ( LOWORD( wparam ) == VK_SPACE || LOWORD( wparam ) == 0x57 || LOWORD( wparam ) == 0x41 || LOWORD( wparam ) == 0x53 || LOWORD( wparam ) == 0x44 )
                    allow_input_processing = false;
            } break;
        }
    } else
        allow_input_processing = false;


    if ( allow_input_processing )
        return 1L;

    return ::CallWindowProcA( original, hwnd, msg, wparam, lparam );
}

void hooks::wnd_proc::init( ) {
    original = reinterpret_cast< WNDPROC >( SetWindowLongPtrA( g_interfaces.window_handle, GWLP_WNDPROC,
                                                              LONG_PTR( wnd_proc::hook ) ) );
}
