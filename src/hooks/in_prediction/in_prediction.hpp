#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace in_prediction
    {
        inline SafetyHookInline original = { };

        bool __fastcall hook( REGISTERS );

        void init( );
    }
}// namespace hooks