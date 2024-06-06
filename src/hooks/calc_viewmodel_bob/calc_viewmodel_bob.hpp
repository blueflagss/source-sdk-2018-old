#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace calc_viewmodel_bob
    {
        inline SafetyHookInline original = { };

        float __fastcall hook( REGISTERS );

        void init( );
    }// namespace calc_viewmodel_bob
}// namespace hooks
