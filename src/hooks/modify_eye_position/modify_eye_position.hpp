#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace modify_eye_position
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, vector_3d &eye_position );

        void init( );
    }// namespace modify_eye_position
}// namespace hooks
