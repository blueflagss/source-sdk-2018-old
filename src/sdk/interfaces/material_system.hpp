#pragma once
#include <utils/utils.hpp>

typedef void ( *materialbufferreleasefunc_t )( );
typedef void ( *materialbufferrestorefunc_t )( int nchangeflags );
typedef void ( *modechangecallbackfunc_t )( void );

#define overbright 2.0f
#define oo_overbright ( 1.0f / 2.0f )
#define gamma 2.2f
#define texgamma 2.2f

class i_material;
class i_mesh;
class i_vertex_buffer;
class i_index_buffer;
struct material_system_config_t;
class v_matrix;
struct hdr_type_t;
class i_material_system_hardware_config;
struct materialhandle_t;
struct material_render_target_depth_t;
struct flashlightstate_t;
class i_texture;
class i_material_proxy_factory;
struct colorcorrectionhandle_t;
class itexturecompositor;
struct occlusionqueryobjecthandle_t;
struct rendertargetsizemode_t;
struct materialsystemhardwareidentifier_t;
class keyvalues;
struct rect_t;
struct lightdesc_t;
class vector4d;
class ishader;
class ivertextexture;
class imorph;
class imatrendercontext;
class icallqueue;
struct morphweight_t;
class ifilelist;

enum shaderparamtype_t {
    shader_param_type_texture,
    shader_param_type_integer,
    shader_param_type_color,
    shader_param_type_vec2,
    shader_param_type_vec3,
    shader_param_type_vec4,
    shader_param_type_envmap,
    shader_param_type_float,
    shader_param_type_bool,
    shader_param_type_fourcc,
    shader_param_type_matrix,
    shader_param_type_material,
    shader_param_type_string,
    shader_param_type_matrix4x2
};

enum materialmatrixmode_t {
    material_view = 0,
    material_projection,
    material_texture0,
    material_texture1,
    material_texture2,
    material_texture3,
    material_texture4,
    material_texture5,
    material_texture6,
    material_texture7,
    material_model,
    num_matrix_modes = material_model + 1,
    num_texture_transforms = material_texture7 - material_texture0 + 1
};

const int num_model_transforms = 53;
const int material_model_max = material_model + num_model_transforms;

enum materialprimitivetype_t {
    material_points = 0x0,
    material_lines,
    material_triangles,
    material_triangle_strip,
    material_line_strip,
    material_line_loop,
    material_polygon,
    material_quads,
    material_instanced_quads,
    material_heterogenous
};

enum materialpropertyopacitytypes_t {
    material_alphatest = 0,
    material_opaque,
    material_translucent
};

enum materialbuffertypes_t {
    material_front = 0,
    material_back
};

enum materialcullmode_t {
    material_cullmode_ccw,
    material_cullmode_cw
};

enum materialindexformat_t {
    material_index_format_unknown = -1,
    material_index_format_16bit = 0,
    material_index_format_32bit,
};

enum materialfogmode_t {
    material_fog_none,
    material_fog_linear,
    material_fog_linear_below_fog_z,
};

enum materialheightclipmode_t {
    material_heightclipmode_disable,
    material_heightclipmode_render_above_height,
    material_heightclipmode_render_below_height
};

enum materialnoninteractivemode_t {
    material_non_interactive_mode_none = -1,
    material_non_interactive_mode_startup = 0,
    material_non_interactive_mode_level_load,

    material_non_interactive_mode_count,
};

#define material_morph_decal ( ( imorph * ) 1 )

enum materialthreadmode_t {
    material_single_threaded,
    material_queued_single_threaded,
    material_queued_threaded
};

enum materialcontexttype_t {
    material_hardware_context,
    material_queued_context,
    material_null_context
};

enum materialfindcontext_t {
    material_findcontext_none,
    material_findcontext_isonamodel,
};

#define createrendertargetflags_hdr 0x00000001
#define createrendertargetflags_automipmap 0x00000002
#define createrendertargetflags_unfilterable_ok 0x00000004
#define createrendertargetflags_noedram 0x00000008
#define createrendertargetflags_temp 0x00000010

enum stenciloperation_t {
    stenciloperation_keep = 1,
    stenciloperation_zero = 2,
    stenciloperation_replace = 3,
    stenciloperation_incrsat = 4,
    stenciloperation_decrsat = 5,
    stenciloperation_invert = 6,
    stenciloperation_incr = 7,
    stenciloperation_decr = 8,
    stenciloperation_force_dword = 0x7fffffff
};

enum stencilcomparisonfunction_t {
    stencilcomparisonfunction_never = 1,
    stencilcomparisonfunction_less = 2,
    stencilcomparisonfunction_equal = 3,
    stencilcomparisonfunction_lessequal = 4,
    stencilcomparisonfunction_greater = 5,
    stencilcomparisonfunction_notequal = 6,
    stencilcomparisonfunction_greaterequal = 7,
    stencilcomparisonfunction_always = 8,
    stencilcomparisonfunction_force_dword = 0x7fffffff
};

enum morphformatflags_t {
    morph_position = 0x0001,
    morph_normal = 0x0002,
    morph_wrinkle = 0x0004,
    morph_speed = 0x0008,
    morph_side = 0x0010,
};

enum standardlightmap_t {
    material_system_lightmap_page_white = -1,
    material_system_lightmap_page_white_bump = -2,
    material_system_lightmap_page_user_defined = -3
};

struct materialsystem_sortinfo_t {
    i_material *material;
    int lightmappageid;
};

#define max_fb_textures 4

enum {
    material_adapter_name_length = 512
};

struct materialadapterinfo_t {
    char m_pdrivername[ material_adapter_name_length ];
    unsigned int m_vendorid;
    unsigned int m_deviceid;
    unsigned int m_subsysid;
    unsigned int m_revision;
    int m_ndxsupportlevel;
    int m_nmaxdxsupportlevel;
    unsigned int m_ndriverversionhigh;
    unsigned int m_ndriverversionlow;
};

enum image_format : int {
    image_format_unknown = -1,
    image_format_rgba8888 = 0,
    image_format_abgr8888,
    image_format_rgb888,
    image_format_bgr888,
    image_format_rgb565,
    image_format_i8,
    image_format_ia88,
    image_format_p8,
    image_format_a8,
    image_format_rgb888_bluescreen,
    image_format_bgr888_bluescreen,
    image_format_argb8888,
    image_format_bgra8888,
    image_format_dxt1,
    image_format_dxt3,
    image_format_dxt5,
    image_format_bgrx8888,
    image_format_bgr565,
    image_format_bgrx5551,
    image_format_bgra4444,
    image_format_dxt1_onebitalpha,
    image_format_bgra5551,
    image_format_uv88,
    image_format_uvwq8888,
    image_format_rgba16161616f,
    image_format_rgba16161616,
    image_format_uvlx8888,
    image_format_r32f,
    image_format_rgb323232f,
    image_format_rgba32323232f,
    image_format_nv_dst16,
    image_format_nv_dst24,
    image_format_nv_intz,
    image_format_nv_rawz,
    image_format_ati_dst16,
    image_format_ati_dst24,
    image_format_nv_null,
    image_format_ati2n,
    image_format_ati1n,
    image_format_dxt1_runtime,
    image_format_dxt5_runtime,
    num_image_formats
};

struct materialvideomode_t {
    int width;
    int height;
    image_format format;
    int refresh_rate;
};

class i_ref_counted {
public:
    virtual int add_ref( ) = 0;
    virtual int release( ) = 0;
};

class i_mat_render_context : public i_ref_counted {
public:
    virtual void begin_render( ) = 0;
    virtual void end_render( ) = 0;
    virtual void flush( bool flush_hardware = false ) = 0;
    virtual void bind_local_cubemap( i_texture *texture ) = 0;
    virtual void set_render_target( i_texture *texture ) = 0;
    virtual i_texture *get_render_target( void ) = 0;
    virtual void get_render_target_dimensions( int &width, int &height ) const = 0;
    virtual void bind( i_material *material, void *proxy_data = 0 ) = 0;
    virtual void bind_light_map_page( int lightmap_page_id ) = 0;
    virtual void depth_range( float znear, float zfar ) = 0;
};

typedef void *( *create_interface_fn )( const char *name, int *return_code );

class i_material_system : public i_app_system {
public:
    uint16_t first_material( ) {
        return utils::get_method< uint16_t( __thiscall * )( void * ) >( this, 86 )( this );
    }

    uint16_t next_material( uint16_t handle ) {
        return utils::get_method< uint16_t( __thiscall * )( void *, uint16_t ) >( this, 87 )( this, handle );
    }

    uint16_t invalid_material( ) {
        return utils::get_method< uint16_t( __thiscall * )( void * ) >( this, 88 )( this );
    }

    i_material *get_material( uint16_t handle ) {
        return utils::get_method< i_material *( __thiscall * ) ( void *, uint16_t ) >( this, 89 )( this, handle );
    }

    i_material *find_material( const char *name, char *type ) {
        return utils::get_method< i_material *( __thiscall * ) ( void *, const char *, char *, bool, void * ) >( this, 84 )( this, name, type, 1, 0 );
    }

    i_material *create_material( const char *material_name, key_values *key_value ) {
        return utils::get_method< i_material *( __thiscall * ) ( void *, const char *, key_values * ) >( this, 83 )( this, material_name, key_value );
    }

    i_mat_render_context *get_render_context( ) {
        return utils::get_method< i_mat_render_context *( __thiscall * ) ( void * ) >( this, 100 )( this );
    }
};