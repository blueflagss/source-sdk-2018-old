#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace do_post_screen_space_effects
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, const c_view_setup *setup );

        void init( );
    }// namespace do_post_screen_space_effects
}// namespace hooks
