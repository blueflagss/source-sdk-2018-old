#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace cvar_get_bool
    {
        namespace sv_cheats
        {
            inline SafetyHookInline original = { };

            bool __fastcall hook( REGISTERS );
        }// namespace sv_cheats

        namespace r_drawallrenderables
        {
            inline SafetyHookInline original = { };

            bool __fastcall hook( REGISTERS );
        }// namespace r_drawallrenderables

        void init( );
    }// namespace cvar_get_bool
}// namespace hooks