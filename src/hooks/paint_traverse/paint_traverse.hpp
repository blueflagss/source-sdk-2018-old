#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace paint_traverse
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, unsigned int panel, bool force_repaint, bool allow_force );

        void init( );
    }// namespace paint_traverse
}// namespace hooks