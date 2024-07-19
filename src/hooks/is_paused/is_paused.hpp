#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace is_paused
    {
        inline SafetyHookInline original = { };

        bool __fastcall hook( REGISTERS );

        void init( );
    }// namespace is_paused
}// namespace hooks
