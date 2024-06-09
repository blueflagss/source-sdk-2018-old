#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace update_animation_state
    {
        inline SafetyHookInline original = { };

        void __vectorcall hook( void *a1, void *a2, float a3, float a4, float a5, void *a6 );

        void init( );
    }// namespace update_animation_state
}// namespace hooks