#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace cl_read_packets
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( bool a1 );

        void init( );
    }
}