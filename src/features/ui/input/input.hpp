#pragma once
#include <globals.hpp>

namespace penumbra
{
    namespace input
    {
        inline glm::vec2 mouse_position;
        extern std::array< bool, 256 > key_states;
        extern std::array< bool, 256 > previous_key_states;

        void update( const std::string &window_name );
        LRESULT wnd_proc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );
        bool in_region( int x, int y, int w, int h );
        bool in_region( glm::vec2 pos, glm::vec2 size );
        bool in_region( glm::vec4 region );
        bool key_pressed( int vkey );
        bool key_released( int vkey );
        bool key_down( int vkey );

        const glm::vec2 &get_mouse_position( );
    }// namespace input
}// namespace gui