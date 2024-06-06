#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace create_move
    {
        inline SafetyHookInline original = { };

        bool __fastcall hook( REGISTERS, float input_sample_time, c_user_cmd *cmd );

        void init( );
    }// namespace create_move
}// namespace hooks
