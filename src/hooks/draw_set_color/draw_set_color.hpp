#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace draw_set_color
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, int r, int g, int b, int a );

        void init( );
    }// namespace draw_set_color
}// namespace hooks
