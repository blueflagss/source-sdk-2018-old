#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace setup_bones
    {
        inline SafetyHookInline original = { };

        bool __fastcall hook( REGISTERS, matrix_3x4 *out, int max, int mask, float curtime );

        void init( );
    }// namespace setup_bones
}// namespace hooks