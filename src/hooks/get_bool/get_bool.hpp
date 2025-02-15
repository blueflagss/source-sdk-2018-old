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

        namespace net_earliertempents
        {
            inline SafetyHookInline original = { };

            bool __fastcall hook( REGISTERS );
        }// namespace net_earliertempents

        namespace net_showfragments
        {
            inline SafetyHookInline original = { };

            bool __fastcall hook( REGISTERS );
        }// namespace net_showfragments

        void init( );
    }// namespace cvar_get_bool
}// namespace hooks