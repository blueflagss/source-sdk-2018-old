#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace standard_blending_rules
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, c_studio_hdr *hdr, vector_3d pos[ ], quat_aligned q[ ], float current_time, int bone_mask );

        void init( );
    }// namespace standard_blending_rules
}// namespace hooks
