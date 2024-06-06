#pragma once
#include <utils/utils.hpp>

struct mstudioanimdesc_t;
struct mstudioseqdesc_t;
struct model_t;
class i_client_renderable;
struct c_studio_hdr;
struct studio_hw_data_t;
struct studio_decal_handle_t;
struct model_instance_handle_t;
class i_material;
class c_studio_hdr;

struct draw_model_state_t {
    c_studio_hdr *studio_hdr;
    void *studio_hw_data;
    i_client_renderable *renderable;
    const matrix_3x4 *model_to_world;
    uint32_t decals;
    int draw_flags;
    int lod;
};

struct model_render_info_t {
    vector_3d origin;
    vector_3d angles;
    i_client_renderable *renderable;
    const model_t *model;
    const matrix_3x4 *model_to_world;
    const matrix_3x4 *lighting_offset;
    const vector_3d *lighting_origin;
    int flags;
    int entity_index;
    int skin;
    int body;
    int hitbox_set;
    unsigned short instance;

    model_render_info_t( ) {
        model_to_world = NULL;
        lighting_offset = NULL;
        lighting_origin = NULL;
    }
};

struct static_prop_render_info_t {
    const matrix_3x4 *model_to_world;
    const model_t *pmodel;
    i_client_renderable *renderable;
    vector_3d *lighting_origin;
    short skin;
    unsigned short instance;
};

struct light_cache_handle_t;
struct ray_t;
struct draw_model_info_t;

enum override_type : int {
    normal = 0,
    build_shadows,
    depth_write,
    ssao_depth_write,
};

enum {
    adddecal_to_all_lods = -1
};

class i_model_render {
public:
    virtual int draw_model( int flags, i_client_renderable *renderable, model_instance_handle_t instance, int entity_index, const model_t *model, vector_3d const &origin, vector_3d const &angles, int skin, int body, int hitbox_set, const matrix_3x4 *model_to_world = NULL, const matrix_3x4 *lighting_offset = NULL ) = 0;
    virtual void forced_material_override( i_material *new_material, override_type type = override_type::normal, int material_index = -1 ) = 0;
    virtual bool is_forced_material_override( ) = 0;
    virtual void set_view_target( const c_studio_hdr *studio_hdr, int body_index, const vector_3d &target ) = 0;
    virtual void *unused_001( );
    virtual void destroy_instance( model_instance_handle_t handle ) = 0;
    virtual void set_static_lighting( model_instance_handle_t handle, void *p_handle ) = 0;
    virtual light_cache_handle_t get_static_lighting( model_instance_handle_t handle ) = 0;
    virtual bool change_instance( model_instance_handle_t handle, i_client_renderable *renderable ) = 0;
    virtual void add_decal( model_instance_handle_t handle, ray_t const &ray, vector_3d const &decal_up, int decal_index, int body, bool no_poke_thru = false, int max_lod_to_decal = adddecal_to_all_lods ) = 0;
    virtual void remove_all_decals( model_instance_handle_t handle ) = 0;
    virtual void remove_all_decals_from_all_models( ) = 0;
    virtual matrix_3x4 *draw_model_shadow_setup( i_client_renderable *renderable, int body, int skin, draw_model_info_t *info, matrix_3x4 *custom_bone_to_world = NULL ) = 0;
    virtual void draw_model_shadow( i_client_renderable *renderable, const draw_model_info_t &info, matrix_3x4 *custom_bone_to_world = NULL ) = 0;
    virtual bool recompute_static_lighting( model_instance_handle_t handle ) = 0;
    virtual void release_all_static_prop_color_data( void ) = 0;
    virtual void restore_all_static_prop_color_data( void ) = 0;
    virtual int draw_model_ex( model_render_info_t &pinfo ) = 0;
    virtual int draw_model_ex_static_prop( model_render_info_t &pinfo ) = 0;
    virtual bool draw_model_setup( model_render_info_t &pinfo, draw_model_state_t *state, matrix_3x4 *custom_bone_to_world, matrix_3x4 **bone_to_world_out ) = 0;
    virtual void draw_model_execute( const draw_model_state_t &state, const model_render_info_t &info, matrix_3x4 *custom_bone_to_world = NULL ) = 0;
    virtual void setup_lighting( const vector_3d &center ) = 0;
    virtual int draw_static_prop_array_fast( static_prop_render_info_t *props, int count, bool shadow_depth ) = 0;
    virtual void suppress_engine_lighting( bool suppress ) = 0;
    virtual void setup_color_meshes( int total_verts ) = 0;
    virtual void add_colored_decal( model_instance_handle_t handle, ray_t const &ray, vector_3d const &decal_up, int decal_index, int body, color c_color, bool no_poke_thru = false, int max_lod_to_decal = adddecal_to_all_lods ) = 0;
    virtual void get_material_override( i_material **out_forced_material, override_type *type ) = 0;
};

#pragma once
