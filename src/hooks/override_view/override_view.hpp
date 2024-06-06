#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace override_view
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, c_view_setup *setup );

        void init( );
    }// namespace override_view
}// namespace hooks
