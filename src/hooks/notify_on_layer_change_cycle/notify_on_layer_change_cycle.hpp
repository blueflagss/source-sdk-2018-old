#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace notify_on_layer_change_cycle
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, const c_animation_layer *layer, const float new_cycle );

        void init( );
    }// namespace notify_on_layer_change_cycle
}// namespace hooks
