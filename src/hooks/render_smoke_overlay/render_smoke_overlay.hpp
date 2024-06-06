#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace render_smoke_overlay
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, bool pre_view_model ); 

        void init( );
    }// namespace render_smoke_overlay
}// namespace hooks
