#pragma once
#include <globals.hpp>

class chams {
public:
    void init( );
    void post_screen_space_effects( c_base_entity *entity );
    void draw_model( i_material *material, c_base_entity *entity, color col, bool ignore_z );
    bool draw_model_execute( i_model_render *model_render, void *edx, void *render_context, const draw_model_state_t &state, const model_render_info_t &info, matrix_3x4 *bone_to_world, c_base_entity *entity );

    bool forced_material = false;

    i_material *flat = nullptr;
    i_material *textured = nullptr;
    i_material *selected = nullptr;

private:

};

inline chams g_chams = { };