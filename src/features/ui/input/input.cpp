#include "input.hpp"
#include <features/ui/elements/object/object.hpp>
#include <features/ui/window_setup.hpp>

std::array< bool, 256 > penumbra::input::key_states = { };
std::array< bool, 256 > penumbra::input::previous_key_states = { };

void penumbra::input::update( const std::string &class_name ) {
    auto window = FindWindowA( class_name.c_str( ), nullptr );

    if ( window != GetForegroundWindow( ) ) {
        return;
    }

    POINT point;

    for ( int i = 0; i < 256u; i++ ) {
        previous_key_states[ i ] = key_states[ i ];
        key_states[ i ] = GetAsyncKeyState( i );
    }

    GetCursorPos( &point );
    ScreenToClient( window, &point );

    mouse_position = { static_cast< float >( point.x ), static_cast< float >( point.y ) };
}

LRESULT penumbra::input::wnd_proc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam ) {
    switch ( msg ) {
        case WM_MOUSEWHEEL:
            penumbra::globals::scroll_delta += GET_WHEEL_DELTA_WPARAM( wparam ) > 0 ? -1.0 : 1.0;
            break;
    }

    return true;
}

bool penumbra::input::in_region( int x, int y, int w, int h ) {
    return mouse_position.x > x && mouse_position.y > y && mouse_position.x < x + w && mouse_position.y < y + h;
}

bool penumbra::input::in_region( glm::vec4 region ) {
    return in_region( region.x, region.y, region.z, region.w );
}

bool penumbra::input::in_region( glm::vec2 pos, glm::vec2 size ) {
    return in_region( pos.x, pos.y, size.x, size.y );
}

bool penumbra::input::key_pressed( int vkey ) {
    return key_states[ vkey ] && !previous_key_states[ vkey ];
}

bool penumbra::input::key_released( int vkey ) {
    return !key_states[ vkey ] && previous_key_states[ vkey ];
}

bool penumbra::input::key_down( int vkey ) {
    return key_states[ vkey ];
}

const glm::vec2 &penumbra::input::get_mouse_position( ) {
    return mouse_position;
}