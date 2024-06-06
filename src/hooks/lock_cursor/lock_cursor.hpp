#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace lock_cursor
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS );

        void init( );
    }// namespace lock_cursor
}// namespace hooks