#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace packet_start
    {
        inline SafetyHookInline original = { };

        int __fastcall hook( REGISTERS, int incoming_sequence, int outgoing_acknowledged );

        void init( );
    }// namespace packet_start
}// namespace hooks