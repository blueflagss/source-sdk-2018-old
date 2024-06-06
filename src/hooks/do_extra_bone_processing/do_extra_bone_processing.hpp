#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace do_extra_bone_processing
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, c_studio_hdr *hdr, vector_3d pos[ ], quat_aligned q[ ], float current_time, int bone_mask );

        void init( );
    }// namespace do_extra_bone_processing
}// namespace hooks
