#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace list_leaves_in_box
    {
        inline SafetyHookInline original = { };

        int __fastcall hook( REGISTERS, const vector_3d &mins, const vector_3d &maxs, unsigned short *list, int list_max );

        void init( );
    }// namespace list_leaves_in_box
}// namespace hooks