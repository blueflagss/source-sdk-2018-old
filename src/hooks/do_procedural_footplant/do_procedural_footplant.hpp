#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace do_procedural_footplant
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, matrix_3x4_aligned bone_to_world[ ], void *left_foot_chain, void *right_foot_chain, vector_3d pos[ ] );

        void init( );
    }// namespace do_procedural_footplant
}// namespace hooks
