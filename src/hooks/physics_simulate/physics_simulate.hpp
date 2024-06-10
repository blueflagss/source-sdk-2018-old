#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace physics_simulate
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS );

        void init( );
    }// namespace physics_simulate
}// namespace hooks