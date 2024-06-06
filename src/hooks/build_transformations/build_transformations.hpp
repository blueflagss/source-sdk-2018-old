#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace build_transformations
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, c_studio_hdr *hdr, const vector_3d &pos, void *quaternion, matrix_3x4_aligned const &matrix, uint32_t mask, bool *computed );

        void init( );
    }// namespace build_transformations
}// namespace hooks
