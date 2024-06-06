#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace draw_model_execute
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, void *render_context, const draw_model_state_t &state, const model_render_info_t &info, matrix_3x4 *bone_to_world );

        void init( );
    }// namespace draw_model_execute
}// namespace hooks