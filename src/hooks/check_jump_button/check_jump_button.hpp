#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace check_jump_button
    {
        inline SafetyHookInline original = { };

        bool hook( REGISTERS );

        void init( );
    }// namespace check_jump_button
}// namespace hooks