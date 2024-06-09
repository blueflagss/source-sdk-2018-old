#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace notify_on_layer_change_weight
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, const c_animation_layer *layer, const float new_weight );

        void init( );
    }// namespace notify_on_layer_change_weight
}// namespace hooks
