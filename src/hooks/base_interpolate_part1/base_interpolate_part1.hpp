#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace base_interpolate_part1
    {
        inline SafetyHookInline original = { };

        int __fastcall hook( REGISTERS, float &curtime, vector_3d &old_origin, vector_3d &old_angs, int &no_more_changes );

        void init( );
    }// namespace base_interpolate_part1
}// namespace hooks
