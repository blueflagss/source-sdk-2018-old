#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace is_hltv
    {
        inline SafetyHookInline original = { };

        bool __fastcall hook( REGISTERS );

        void init( );
    }// namespace is_hltv
}// namespace hooks
