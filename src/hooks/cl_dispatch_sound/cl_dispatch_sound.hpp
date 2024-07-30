#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace cl_dispatch_sound
    {
        inline SafetyHookInline original = { };

        void __cdecl hook( const soundinfo_t &sound );

        void init( );
    }// namespace cl_dispatch_sound
}// namespace hooks