#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace level_shutdown
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS );

        void init( );
    }// namespace level_shutdown
}// namespace hooks
