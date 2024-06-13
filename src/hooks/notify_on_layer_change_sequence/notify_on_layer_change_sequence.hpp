#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace notify_on_layer_change_sequence
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, const c_animation_layer *layer, const float new_sequence );

        void init( );
    }// namespace notify_on_layer_change_sequence
}// namespace hooks
