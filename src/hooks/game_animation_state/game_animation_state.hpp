#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace setup_velocity
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS );
    };// namespace setup_velocity

    namespace setup_movement
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS );
    };// namespace setup_movement

    namespace setup_alive_loop
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS );
    };// namespace setup_alive_loop

    namespace anim_state
    {
        void init( );
    };
}// namespace hooks