#pragma once
#include <utils/utils.hpp>

class i_material;
class i_client_renderable;

struct model_t;
struct vcollide_t;
struct trace_t;
struct virtualmodel_t;

struct mstudiobone_t {
    int sznameindex;
    inline char *const name( void ) const { return ( ( char * ) this ) + sznameindex; }
    int parent;
    int bonecontroller[ 6 ];
    vector_3d pos;
    vector_4d quat;
    vector_3d rot;
    vector_3d posscale;
    vector_3d rotscale;
    matrix_3x4 pose_to_bone;
    vector_4d alignment;
    int flags;
    int proctype;
    int procindex;
    mutable int physicsbone;
    int surfacepropidx;
    inline char *const surface_prop( void ) const { return ( ( char * ) this ) + surfacepropidx; }
    int contents;
    int unused[ 8 ];
};

struct mstudiobbox_t {
    int bone;       // 0x0000
    int group;      // 0x0004
    vector_3d min;  // 0x0008
    vector_3d max;  // 0x0014
    int name_id;    // 0x0020
    vector_3d angle;// 0x0024
    float radius;   // 0x0030
    PAD( 0x10 );    // 0x0034

    inline const char *get_hitbox_name( ) {
        if ( name_id == 0 )
            return "";

        return reinterpret_cast< const char * >( reinterpret_cast< std::uint8_t * >( const_cast< mstudiobbox_t * >( this ) ) + this->name_id );
    }

    mstudiobbox_t( ) { }
};

struct mstudiohitboxset_t {
    int name_index;
    inline char *const name( void ) const { return ( ( char * ) this ) + name_index; }
    int num_hitboxes;
    int hitbox_index;

    inline mstudiobbox_t *hitbox( int i ) const { return ( mstudiobbox_t * ) ( ( ( unsigned char * ) this ) + hitbox_index ) + i; };
};

struct studiohdr_t {
    int id;
    int version;
    int checksum;
    char name[ 64 ];
    int length;
    vector_3d eye_position;
    vector_3d illum_position;
    vector_3d hull_min;
    vector_3d hull_max;
    vector_3d view_min;
    vector_3d view_max;
    int flags;
    int numbones;
    int boneindex;
    int numbonecontrollers;
    int bonecontrollerindex;
    int numhitboxsets;
    int hitboxsetindex;

    inline mstudiobone_t *bone( int i ) const { return ( mstudiobone_t * ) ( ( ( char * ) this ) + boneindex ) + i; };
    inline mstudiohitboxset_t *hitbox_set( const int n ) const { return ( mstudiohitboxset_t * ) ( ( ( unsigned char * ) this ) + hitboxsetindex ) + n; };
    inline mstudiobbox_t* hitbox(int i, int set) const {
        mstudiohitboxset_t const *s = hitbox_set( set );
        if ( !s )
            return nullptr;

        return s->hitbox( i );
    }
};

class c_studio_hdr {
public:
    studiohdr_t *studio_hdr;
    void *unk0;
};

class c_model_info {
public:
    virtual ~c_model_info( void ) { }
    virtual const model_t *get_model( int index ) = 0;
    virtual int get_model_index( const char *name ) const = 0;
    virtual const char *get_model_name( const model_t *model ) const = 0;
    virtual vcollide_t *get_vcollide( const model_t *model ) = 0;
    virtual vcollide_t *get_vcollide( int index ) = 0;
    virtual void get_model_bounds( const model_t *model, vector_3d &mins, vector_3d &maxs ) const = 0;
    virtual void get_model_renderbounds( const model_t *model, vector_3d &mins, vector_3d &maxs ) const = 0;
    virtual int get_model_framecount( const model_t *model ) const = 0;
    virtual int get_model_type( const model_t *model ) const = 0;
    virtual void *get_model_extra_data( const model_t *model ) = 0;
    virtual bool model_has_material_proxy( const model_t *model ) const = 0;
    virtual bool is_translucent( model_t const *model ) const = 0;
    virtual bool is_translucent_two_pass( const model_t *model ) const = 0;
    virtual void recompute_translucency( const model_t *model, int skin, int body, void *client_renderable, float instance_alpha_modulate = 1.0f ) = 0;
    virtual int get_model_material_count( const model_t *model ) const = 0;
    virtual void get_model_materials( const model_t *model, int count, i_material **material ) = 0;
    virtual bool is_model_vertexlit( const model_t *model ) const = 0;
    virtual const char *get_model_keyvalue_text( const model_t *model ) = 0;
    virtual bool get_model_keyvalue( const model_t *model, void *buf ) = 0;
    virtual float get_model_radius( const model_t *model ) = 0;
    virtual const studiohdr_t *find_model( const studiohdr_t *pstudiohdr, void **cache, const char *model_name ) const = 0;
    virtual const studiohdr_t *find_model( void *cache ) const = 0;
    virtual virtualmodel_t *get_virtual_model( const studiohdr_t *pstudiohdr ) const = 0;
    virtual char *get_anim_block( const studiohdr_t *pstudiohdr, int iblock ) const = 0;
    virtual void *pad001( ) const = 0;
    virtual void *pad002( ) const = 0;
    virtual void get_model_material_color_and_lighting( const model_t *model, vector_3d const &origin, vector_3d const &angles, trace_t *trace, vector_3d &lighting, vector_3d &material_color ) = 0;
    virtual void get_illumination_point( const model_t *model, i_client_renderable *renderable, vector_3d const &origin, vector_3d const &angles, vector_3d *lighting_center ) = 0;
    virtual int get_model_contents( int modelindex ) = 0;
    virtual studiohdr_t *get_studio_model( const model_t *model ) = 0;
};

#pragma once