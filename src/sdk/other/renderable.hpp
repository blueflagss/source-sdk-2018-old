#pragma once

struct matrix_3x4;
struct model_instance_handle_t;
struct shadow_type_t;
struct client_shadow_handle_t;
struct client_render_handle_t;

class i_client_unknown;
class i_pvs_notify;
class model_t;

struct renderable_instance_t {
    uint8_t alpha;

    __forceinline renderable_instance_t( ) : alpha{ 255ui8 } {}
};

class i_client_renderable {
public:
    virtual i_client_unknown *get_client_unknown( ) = 0;
    virtual vector_3d const &get_render_origin( void ) = 0;
    virtual vector_3d const &get_render_angles( void ) = 0;
    virtual bool should_draw( void ) = 0;
    virtual void *pad001( void ) = 0;
    virtual void *pad002( void ) = 0;
    virtual client_shadow_handle_t get_shadow_handle( ) const = 0;
    virtual client_render_handle_t &render_handle( ) = 0;
    virtual const model_t *get_model( ) const = 0;
    virtual int draw_model( int flags, const renderable_instance_t &instance = { } ) = 0;
    virtual int get_body( ) = 0;
    virtual void get_color_modulation( float *color ) = 0;
    virtual bool lod_test( ) = 0;
    virtual bool setup_bones( matrix_3x4 *out, int bones, int mask, float curtime ) = 0;
    virtual void setup_weights( const matrix_3x4 *out, int flex_weight_count, float *flex_weights, float *flex_delayed_weights ) = 0;
    virtual void do_animation_events( void ) = 0;
    virtual i_pvs_notify *get_pvs_notify_interface( ) = 0;
    virtual void get_render_bounds( vector_3d &mins, vector_3d &maxs ) = 0;
    virtual void get_render_bounds_worldspace( vector_3d &mins, vector_3d &maxs ) = 0;
    virtual void get_shadow_render_bounds( vector_3d &mins, vector_3d &maxs, shadow_type_t shadow_type ) = 0;
    virtual bool should_receive_projected_textures( int flags ) = 0;
    virtual bool get_shadow_cast_distance( float *dist, shadow_type_t shadow_type ) const = 0;
    virtual bool get_shadow_cast_direction( vector_3d *direction, shadow_type_t shadow_type ) const = 0;
    virtual bool is_show_dirty( ) = 0;
    virtual void mark_shadow_dirty( bool bDirty ) = 0;
    virtual i_client_renderable *get_shadow_parent( ) = 0;
    virtual i_client_renderable *first_shadow_child( ) = 0;
    virtual i_client_renderable *next_shadow_peer( ) = 0;
    virtual shadow_type_t shadow_cast_type( ) = 0;
    virtual void unused2( ){ };
    virtual void create_model_instance( ) = 0;
    virtual model_instance_handle_t get_model_instance( ) = 0;
    virtual const matrix_3x4 &renderable_to_world_transform( ) = 0;
    virtual int lookup_attachment( const char *attachment_name ) = 0;
    virtual bool get_attachment( int number, vector_3d &origin, vector_3d &angles ) = 0;
    virtual bool get_attachment( int number, matrix_3x4 &matrix ) = 0;
    virtual float *get_render_clip_plane( void ) = 0;
    virtual int get_skin( ) = 0;
    virtual bool is_two_pass( void ) = 0;
    virtual void on_threaded_draw_setup( ) = 0;
    virtual bool uses_flex_delayed_weights( ) = 0;
    virtual void record_tool_message( ) = 0;
    virtual bool Should_Draw_For_Split_Screen_User( int slot ) = 0;
    virtual uint8_t override_alpha_modulation( uint8_t alpha ) = 0;
    virtual uint8_t override_shadow_alpha_modulation( uint8_t alpha ) = 0;
    virtual void *get_client_model_renderable( ) = 0;
};