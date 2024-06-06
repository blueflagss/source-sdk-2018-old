#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace estimate_abs_velocity
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, vector_3d &velocity );

        void init( );
    }// namespace estimate_abs_velocity
}// namespace hooks
