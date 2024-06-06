#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace wnd_proc
    {
        inline WNDPROC original = nullptr;

        LRESULT __stdcall hook( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );

        void init( );
    }// namespace wnd_proc
}// namespace hooks