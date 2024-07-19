#pragma once
#include <utils/utils.hpp>

enum stereo_eye : int {
    stereo_mono = 0,
    stereo_left,
    stereo_right,
    stereo_max,
};

class c_view_setup {
public:
    int x;
    int old_x;
    int y;
    int old_y;
    int width;
    int old_width;
    int height;
    int old_height;
    bool ortho;
    float ortho_left;
    float ortho_top;
    float ortho_right;
    float ortho_bottom;
    bool custom_view_matrix;
    matrix_3x4 custom_matrix;
    PAD( 0x48 );
    float fov;
    float viewmodel_fov;
    vector_3d origin;
    vector_3d angles;
    float znear;
    float zfar;
    float znear_viewmodel;
    float zfar_viewmodel;
    float aspect_ratio;
    float near_blur_depth;
    float near_focus_depth;
    float far_focus_depth;
    float far_blur_depth;
    float near_blur_radius;
    float far_blur_radius;
    int quality;
    int motion_blur_mode;
    float shutter_time;
    vector_3d shutter_open_pos;
    vector_3d shutter_open_angles;
    vector_3d shutter_close_pos;
    vector_3d shutter_close_angles;
    float off_center_top;
    float off_center_bottom;
    float off_center_left;
    float off_center_right;
    int edge_blur;
};

class c_render_view {
public:
    virtual void draw_brush_model( i_client_entity *base_entity, void *model, const vector_3d &origin, const vector_3d &angles, bool unused ) = 0;
    virtual void draw_identity_brush_model( void *list, void *model ) = 0;
    virtual void touch_light( struct dlight_t *light ) = 0;
    virtual void draw_3d_debug_overlays( void ) = 0;
    virtual void set_blend( float blend ) = 0;
    virtual float get_blend( void ) = 0;

    void set_color_modulation( color col ) {
        float color[ ] = { static_cast< float >( col.r ) / 255.0f,
                          static_cast< float >( col.g ) / 255.0f,
                          static_cast< float >( col.b ) / 255.0f,
                          static_cast< float >( col.a ) / 255.0f 
        };

        return utils::get_method< void( __thiscall * )( void *, float * ) >( this, 6 )( this, color );
    }

    void get_matrices_for_view( const c_view_setup &view, v_matrix *world_to_view, v_matrix *view_to_projection, v_matrix *world_to_projection, v_matrix *world_to_pixels ) {
        return utils::get_method< void( __thiscall * )( void *, const c_view_setup &, v_matrix *, v_matrix *, v_matrix *, v_matrix * ) >( this, 56 )( this, view, world_to_view, view_to_projection, world_to_projection, world_to_pixels );
    }

private:
};