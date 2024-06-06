#pragma once
#include <utils/utils.hpp>

class i_material_var;
class i_texture;
class i_material_proxy;
class four_cc;

#define vertex_position 0x0001
#define vertex_normal 0x0002
#define vertex_color 0x0004
#define vertex_specular 0x0008
#define vertex_tangent_s 0x0010
#define vertex_tangent_t 0x0020
#define vertex_tangent_space ( vertex_tangent_s | vertex_tangent_t )
#define vertex_wrinkle 0x0040
#define vertex_bone_index 0x0080
#define vertex_format_vertex_shader 0x0100
#define vertex_format_use_exact_format 0x0200
#define vertex_format_compressed 0x400
#define vertex_last_bit 10
#define vertex_bone_weight_bit ( vertex_last_bit + 1 )
#define user_data_size_bit ( vertex_last_bit + 4 )
#define tex_coord_size_bit ( vertex_last_bit + 7 )
#define vertex_bone_weight_mask ( 0x7 << vertex_bone_weight_bit )
#define user_data_size_mask ( 0x7 << user_data_size_bit )
#define vertex_format_field_mask 0x0ff
#define vertex_format_unknown 0

enum vertex_element : int {
    element_none = -1,
    element_position = 0,
    element_normal = 1,
    element_color = 2,
    element_specular = 3,
    element_tangent_s = 4,
    element_tangent_t = 5,
    element_wrinkle = 6,
    element_boneindex = 7,
    element_boneweights1 = 8,
    element_boneweights2 = 9,
    element_boneweights3 = 10,
    element_boneweights4 = 11,
    element_userdata1 = 12,
    element_userdata2 = 13,
    element_userdata3 = 14,
    element_userdata4 = 15,
    element_texcoord1d_0 = 16,
    element_texcoord1d_1 = 17,
    element_texcoord1d_2 = 18,
    element_texcoord1d_3 = 19,
    element_texcoord1d_4 = 20,
    element_texcoord1d_5 = 21,
    element_texcoord1d_6 = 22,
    element_texcoord1d_7 = 23,
    element_texcoord2d_0 = 24,
    element_texcoord2d_1 = 25,
    element_texcoord2d_2 = 26,
    element_texcoord2d_3 = 27,
    element_texcoord2d_4 = 28,
    element_texcoord2d_5 = 29,
    element_texcoord2d_6 = 30,
    element_texcoord2d_7 = 31,
    element_texcoord3d_0 = 32,
    element_texcoord3d_1 = 33,
    element_texcoord3d_2 = 34,
    element_texcoord3d_3 = 35,
    element_texcoord3d_4 = 36,
    element_texcoord3d_5 = 37,
    element_texcoord3d_6 = 38,
    element_texcoord3d_7 = 39,
    element_texcoord4d_0 = 40,
    element_texcoord4d_1 = 41,
    element_texcoord4d_2 = 42,
    element_texcoord4d_3 = 43,
    element_texcoord4d_4 = 44,
    element_texcoord4d_5 = 45,
    element_texcoord4d_6 = 46,
    element_texcoord4d_7 = 47,
    element_numelements = 48
};

#define compressed_normals_separatetangents_short2 0
#define compressed_normals_combinedtangents_ubyte4 1
#define compressed_normals_type compressed_normals_combinedtangents_ubyte4

enum material_var_flags : int {
    debug = ( 1 << 0 ),
    no_debug_override = ( 1 << 1 ),
    no_draw = ( 1 << 2 ),
    use_in_fillrate_mode = ( 1 << 3 ),
    vertexcolor = ( 1 << 4 ),
    vertexalpha = ( 1 << 5 ),
    selfillum = ( 1 << 6 ),
    additive = ( 1 << 7 ),
    alphatest = ( 1 << 8 ),
    multipass = ( 1 << 9 ),
    znearer = ( 1 << 10 ),
    model = ( 1 << 11 ),
    flat = ( 1 << 12 ),
    nocull = ( 1 << 13 ),
    nofog = ( 1 << 14 ),
    ignore_z = ( 1 << 15 ),
    decal = ( 1 << 16 ),
    envmapsphere = ( 1 << 17 ),
    noalphamod = ( 1 << 18 ),
    envmapcameraspace = ( 1 << 19 ),
    basealphaenvmapmask = ( 1 << 20 ),
    translucent = ( 1 << 21 ),
    normalmapalphaenvmapmask = ( 1 << 22 ),
    needs_software_skinning = ( 1 << 23 ),
    opaquetexture = ( 1 << 24 ),
    envmapmode = ( 1 << 25 ),
    suppress_decals = ( 1 << 26 ),
    halflambert = ( 1 << 27 ),
    wireframe = ( 1 << 28 ),
    allowalphatocoverage = ( 1 << 29 ),
    ignore_alpha_modulation = ( 1 << 30 ),
};

enum material_var_flags2 : int {
    lighting_unlit = 0,
    lighting_vertex_lit = ( 1 << 1 ),
    lighting_lightmap = ( 1 << 2 ),
    lighting_bumped_lightmap = ( 1 << 3 ),
    lighting_mask = ( lighting_vertex_lit | lighting_lightmap | lighting_bumped_lightmap ),
    diffuse_bumpmapped_model = ( 1 << 4 ),
    uses_env_cubemap = ( 1 << 5 ),
    needs_tangent_spaces = ( 1 << 6 ),
    needs_software_lighting = ( 1 << 7 ),
    blend_with_lightmap_alpha = ( 1 << 8 ),
    needs_baked_lighting_snapshots = ( 1 << 9 ),
    use_flashlight = ( 1 << 10 ),
    use_fixed_function_baked_lighting = ( 1 << 11 ),
    needs_fixed_function_flashlight = ( 1 << 12 ),
    use_editor = ( 1 << 13 ),
    needs_power_of_two_frame_buffer_texture = ( 1 << 14 ),
    needs_full_frame_buffer_texture = ( 1 << 15 ),
    is_spritecard = ( 1 << 16 ),
    uses_vertexid = ( 1 << 17 ),
    supports_hw_skinning = ( 1 << 18 ),
    supports_flashlight = ( 1 << 19 ),
};

struct vertex_format_t;
struct material_property_types_t;
struct morph_format_t;

enum preview_image_ret_val : int {
    preview_image_bad = 0,
    preview_image_ok,
    no_preview_image,
};

class i_material {
public:
    virtual const char *get_name( ) const = 0;
    virtual const char *get_texture_group_name( ) const = 0;
    virtual preview_image_ret_val get_preview_image_properties( int *width, int *height, image_format *imageformat, bool *istranslucent ) const = 0;
    virtual preview_image_ret_val get_preview_image( unsigned char *data, int width, int height, image_format imageformat ) const = 0;
    virtual int get_mapping_width( ) = 0;
    virtual int get_mapping_height( ) = 0;
    virtual int get_num_animation_frames( ) = 0;
    virtual bool in_material_page( void ) = 0;
    virtual void get_material_offset( float *poffset ) = 0;
    virtual void get_material_scale( float *pscale ) = 0;
    virtual i_material *get_material_page( void ) = 0;
    virtual i_material_var *find_var( const char *varname, bool *found, bool complain = true ) = 0;
    virtual void increment_reference_count( void ) = 0;
    virtual void decrement_reference_count( void ) = 0;
    virtual int get_enumeration_id( void ) const = 0;
    virtual void get_low_res_color_sample( float s, float t, float *color ) const = 0;
    virtual void recompute_state_snapshots( ) = 0;
    virtual bool is_translucent( ) = 0;
    virtual bool is_alpha_tested( ) = 0;
    virtual bool is_vertex_lit( ) = 0;
    virtual vertex_format_t get_vertex_format( ) const = 0;
    virtual bool has_proxy( void ) const = 0;
    virtual bool uses_envcube_map( void ) = 0;
    virtual bool needs_tangent_space( void ) = 0;
    virtual bool needs_power_of_two_frame_buffer_texture( bool check = true ) = 0;
    virtual bool needs_full_frame_buffer_texture( bool check = true ) = 0;
    virtual bool needs_software_skinning( void ) = 0;
    virtual void alpha_modulate( float alpha ) = 0;
    virtual void color_modulate( float r, float g, float b ) = 0;
    virtual void set_material_var_flag( material_var_flags flag, bool on ) = 0;
    virtual bool get_material_var_flag( material_var_flags flag ) const = 0;
    virtual void get_reflectivity( vector_3d &reflect ) = 0;
    virtual bool get_property_flag( material_property_types_t type ) = 0;
    virtual bool is_two_sided( ) = 0;
    virtual void set_shader( const char *shader_name ) = 0;
    virtual int get_num_passes( void ) = 0;
    virtual int get_texture_memory_bytes( void ) = 0;
    virtual void refresh( ) = 0;
    virtual bool needs_light_map_blend_alpha( void ) = 0;
    virtual bool needs_software_lighting( void ) = 0;
    virtual int shader_param_count( ) const = 0;
    virtual i_material_var **get_shader_params( void ) = 0;
    virtual bool is_error_material( ) const = 0;
    virtual void unused( ) {}
    virtual float get_alpha_modulation( ) = 0;
    virtual void get_color_modulation( float *r, float *g, float *b ) = 0;
    virtual bool is_translucent_under_modulation( float alpha_modulation = 1.0f ) const = 0;
    virtual i_material_var *find_var_fast( char const *var_name, unsigned int *token ) = 0;
    virtual void set_shader_and_params( key_values *key_values ) = 0;
    virtual const char *get_shader_name( ) const = 0;
    virtual void delete_if_unreferenced( ) = 0;
    virtual bool is_sprite_card( ) = 0;
    virtual void call_bind_proxy( void *proxy_data, void *call_queue ) = 0;
    virtual void refresh_preserving_material_vars( ) = 0;
    virtual bool was_reloaded_from_whitelist( ) = 0;
    virtual bool set_temp_excluded( bool set, int excluded_dimension_limit = 0 ) = 0;
    virtual int get_reference_count( ) const = 0;
};

enum material_var_type : int {
    material_var_type_float = 0,
    material_var_type_string,
    material_var_type_vector,
    material_var_type_texture,
    material_var_type_int,
    material_var_type_fourcc,
    material_var_type_undefined,
    material_var_type_matrix,
    material_var_type_material,
};

typedef unsigned short material_var_sym_t;

class i_material_var {
public:
    virtual i_texture *get_texture_value( void ) = 0;
    virtual char const *get_name( void ) const = 0;
    virtual material_var_sym_t get_name_as_symbol( ) const = 0;
    virtual void set_float_value( float val ) = 0;
    virtual void set_int_value( int val ) = 0;
    virtual void set_string_value( char const *val ) = 0;
    virtual char const *get_string_value( void ) const = 0;
    virtual void set_four_cc_value( four_cc type, void *data ) = 0;
    virtual void get_four_cc_value( four_cc *type, void **data ) = 0;
    virtual void set_vec_value( float const *val, int comps ) = 0;
    virtual void set_vec_value( float x, float y ) = 0;
    virtual void set_vec_value( float x, float y, float z ) = 0;
    virtual void set_vec_value( float x, float y, float z, float w ) = 0;
    virtual void get_linear_vec_value( float *val, int comps ) const = 0;
    virtual void set_texture_value( i_texture * ) = 0;
    virtual i_material *get_material_value( void ) = 0;
    virtual void set_material_value( i_material * ) = 0;
    virtual bool is_defined( ) const = 0;
    virtual void set_undefined( ) = 0;
    virtual void set_matrix_value( v_matrix const &matrix ) = 0;
    virtual const v_matrix &get_matrix_value( ) = 0;
    virtual bool matrix_is_identity( ) const = 0;
    virtual void copy_from( i_material_var *var ) = 0;
    virtual void set_value_auto_detect_type( char const *val ) = 0;
    virtual i_material *get_owning_material( ) = 0;
    virtual void set_vec_component_value( float value, int component ) = 0;
    virtual int get_int_value_internal( void ) const = 0;
    virtual float get_float_value_internal( void ) const = 0;
    virtual float const *get_vec_value_internal( ) const = 0;
    virtual void get_vec_value_internal( float *val, int comps ) const = 0;
    virtual int vector_size_internal( ) const = 0;
};