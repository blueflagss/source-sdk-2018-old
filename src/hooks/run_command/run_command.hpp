#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace run_command
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, c_base_player *player, c_user_cmd *ucmd, c_move_helper *move_helper );

        void init( );
    }// namespace run_command
}// namespace hooks
