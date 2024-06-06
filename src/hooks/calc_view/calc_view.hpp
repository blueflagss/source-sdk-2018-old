#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace calc_view
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, vector_3d &eye_origin, vector_3d &eye_angles, float &z_near, float &z_far, float &fov );

        void init( );
    }// namespace calc_view
}// namespace hooks
