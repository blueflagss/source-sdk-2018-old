#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace pre_entity_packet_received
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, int commands_acknowledged, int current_world_update_packet, int server_ticks_elapsed );

        void init( );
    }// namespace pre_entity_packet_received
}// namespace hooks