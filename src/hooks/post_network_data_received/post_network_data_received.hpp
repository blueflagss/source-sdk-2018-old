#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace post_network_data_received
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, int commands_acknowledged );

        void init( );
    }// namespace post_network_data_received
}// namespace hooks