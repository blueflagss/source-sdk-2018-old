#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace send_datagram
    {
        inline bool did_hook = false;

        inline SafetyHookInline original = { };

        int __fastcall hook( REGISTERS, void *datagram );

        void init( );
    }// namespace send_datagram
}// namespace hooks