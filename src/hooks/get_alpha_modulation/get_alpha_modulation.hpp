#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace get_alpha_modulation
    {
        inline SafetyHookInline original = { };

        float __fastcall hook( REGISTERS );

        void init( );
    }
}// namespace hooks