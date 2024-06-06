#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace calc_viewmodel_view
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, c_base_player *owner, const vector_3d &eye_position, const vector_3d &eye_angles );

        void init( );
    }// namespace calc_viewmodel_view
}// namespace hooks
