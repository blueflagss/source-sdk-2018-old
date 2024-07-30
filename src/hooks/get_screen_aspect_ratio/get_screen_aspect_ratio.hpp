#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace get_screen_aspect_ratio
    {
        inline SafetyHookInline original;

        float __fastcall hook( REGISTERS, int viewport_width, int viewport_height );

        void init( );
    }// namespace get_screen_aspect_ratio
}// namespace hooks