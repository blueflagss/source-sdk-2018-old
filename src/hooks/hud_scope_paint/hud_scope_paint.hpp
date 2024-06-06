#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace hud_scope_paint
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS );

        void init( );
    }// namespace hud_scope_paint
}// namespace hooks