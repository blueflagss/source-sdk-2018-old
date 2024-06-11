#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace get_eye_angles
    {
        inline SafetyHookInline original;

        vector_3d &__fastcall hook( REGISTERS );

        void init( );
    }// namespace get_eye_angles
}// namespace hooks