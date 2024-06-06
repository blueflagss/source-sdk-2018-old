#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace frame_stage_notify
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, client_frame_stage stage );

        void init( );
    }// namespace frame_stage_notify
}// namespace hooks