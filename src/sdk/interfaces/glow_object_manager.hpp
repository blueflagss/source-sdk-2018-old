#pragma once

struct glow_object_definition_t {
    bool is_unused( ) const {
        return next_free_slot != -2;
    }

    void set( float r, float g, float b, float a ) {
        glow_color = vector_3d( r, g, b );
        glow_alpha = a;
        render_when_occluded = true;
        render_when_unoccluded = false;
    }

    c_base_entity *entity;
    vector_3d glow_color;
    float glow_alpha;
    bool glow_alpha_capped_by_render_alpha;
    float glow_alpha_function_of_max_velocity;
    float glow_alpha_max;
    float glow_pulse_overdrive;
    bool render_when_occluded;
    bool render_when_unoccluded;
    bool full_bloom_render;
    int full_bloom_stencil_test_value;
    int render_style;
    int splitscreen_slot;
    int next_free_slot;
};

class c_glow_object_mgr {
public:
    c_utl_vector< glow_object_definition_t > glow_object_definitions;
    int first_free_slot;
};