#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace on_screen_size_changed
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, int old_wide, int old_tall );

        void init( );
    }// namespace on_screen_size_changed
}// namespace hooks
