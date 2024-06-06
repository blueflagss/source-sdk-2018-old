#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace process_movement
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, c_base_entity *player, c_move_data *data ); 

        void init( );
    }// namespace process_movement
}// namespace hooks
