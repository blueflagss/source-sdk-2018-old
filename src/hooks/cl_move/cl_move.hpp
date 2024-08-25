#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace cl_move
    {
        inline SafetyHookInline original = { };

        void __vectorcall hook( float accumulated_extra_samples, bool final_tick );

        void init( );
    }// namespace cl_move
}// namespace hooks
