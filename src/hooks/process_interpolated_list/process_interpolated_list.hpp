#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace process_interpolated_list
    {
        inline SafetyHookInline original = { };

        void __cdecl hook( );

        void init( );
    }// namespace process_interpolated_list
}// namespace hooks
