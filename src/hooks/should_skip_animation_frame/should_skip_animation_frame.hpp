#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace should_skip_animation_frame
    {
        inline SafetyHookInline original = { };

        bool __fastcall hook( REGISTERS );

        void init( );
    }// namespace should_skip_animation_frame
}// namespace hooks
