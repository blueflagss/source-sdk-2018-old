#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace cl_fire_events
    {
        inline SafetyHookInline original = { };

        void __cdecl hook( );

        void init( );
    }// namespace cl_fire_events
}// namespace hooks
