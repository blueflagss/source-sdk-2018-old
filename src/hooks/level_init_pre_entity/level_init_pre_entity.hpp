#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace level_init_pre_entity
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, const char *map );

        void init( );
    }// namespace level_init_pre_entity
}// namespace hooks
