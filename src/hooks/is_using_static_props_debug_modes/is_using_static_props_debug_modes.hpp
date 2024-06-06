#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace is_using_static_props_debug_modes
    {
        inline SafetyHookInline original = { };

        bool __cdecl hook( );

        void init( );
    }// namespace is_using_static_props_debug_modes
}// namespace hooks