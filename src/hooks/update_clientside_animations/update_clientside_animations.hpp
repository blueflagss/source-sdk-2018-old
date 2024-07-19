#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace update_clientside_animations
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS );

        void init( );
    }// namespace update_clientside_animations
}// namespace hooks