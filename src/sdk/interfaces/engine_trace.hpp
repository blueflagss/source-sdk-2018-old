#pragma once
#include <globals.hpp>
#include <sdk/interfaces/engine_trace.hpp>
#include <sdk/math/ray.hpp>

#define contents_empty 0
#define contents_solid 0x1
#define contents_window 0x2
#define contents_aux 0x4
#define contents_grate 0x8
#define contents_slime 0x10
#define contents_water 0x20
#define contents_blocklos 0x40
#define contents_opaque 0x80
#define last_visible_contents contents_opaque
#define all_visible_contents ( last_visible_contents | ( last_visible_contents - 1 ) )
#define contents_testfogvolume 0x100
#define contents_unused 0x200
#define contents_blocklight 0x400
#define contents_team1 0x800
#define contents_team2 0x1000
#define contents_ignore_nodraw_opaque 0x2000
#define contents_moveable 0x4000
#define contents_areaportal 0x8000
#define contents_playerclip 0x10000
#define contents_monsterclip 0x20000
#define contents_current_0 0x40000
#define contents_current_90 0x80000
#define contents_current_180 0x100000
#define contents_current_270 0x200000
#define contents_current_up 0x400000
#define contents_current_down 0x800000
#define contents_origin 0x1000000
#define contents_monster 0x2000000
#define contents_debris 0x4000000
#define contents_detail 0x8000000
#define contents_translucent 0x10000000
#define contents_ladder 0x20000000
#define contents_hitbox 0x40000000

enum masks : uint32_t {
    mask_all = ( 0xffffffff ),
    mask_solid = ( contents_solid | contents_moveable | contents_window | contents_monster | contents_grate ),
    mask_playersolid = ( contents_solid | contents_moveable | contents_playerclip | contents_window | contents_monster | contents_grate ),
    mask_npcsolid = ( contents_solid | contents_moveable | contents_monsterclip | contents_window | contents_monster | contents_grate ),
    mask_npcfluid = ( contents_solid | contents_moveable | contents_monsterclip | contents_window | contents_monster ),
    mask_water = ( contents_water | contents_moveable | contents_slime ),
    mask_opaque = ( contents_solid | contents_moveable | contents_opaque ),
    mask_opaque_and_npcs = ( mask_opaque | contents_monster ),
    mask_blocklos = ( contents_solid | contents_moveable | contents_blocklos ),
    mask_blocklos_and_npcs = ( mask_blocklos | contents_monster ),
    mask_visible = ( mask_opaque | contents_ignore_nodraw_opaque ),
    mask_visible_and_npcs = ( mask_opaque_and_npcs | contents_ignore_nodraw_opaque ),
    mask_shot = ( contents_solid | contents_moveable | contents_monster | contents_window | contents_debris | contents_hitbox ),
    mask_shot_brushonly = ( contents_solid | contents_moveable | contents_window | contents_debris ),
    mask_shot_hull = ( contents_solid | contents_moveable | contents_monster | contents_window | contents_debris | contents_grate ),
    mask_shot_portal = ( contents_solid | contents_moveable | contents_window | contents_monster ),
    mask_solid_brushonly = ( contents_solid | contents_moveable | contents_window | contents_grate ),
    mask_playersolid_brushonly = ( contents_solid | contents_moveable | contents_window | contents_playerclip | contents_grate ),
    mask_npcsolid_brushonly = ( contents_solid | contents_moveable | contents_window | contents_monsterclip | contents_grate ),
    mask_npcworldstatic = ( contents_solid | contents_window | contents_monsterclip | contents_grate ),
    mask_npcworldstatic_fluid = ( contents_solid | contents_window | contents_monsterclip ),
    mask_splitareaportal = ( contents_water | contents_slime ),
    mask_current = ( contents_current_0 | contents_current_90 | contents_current_180 | contents_current_270 | contents_current_up | contents_current_down ),
    mask_deadsolid = ( contents_solid | contents_playerclip | contents_window | contents_grate )
};

class i_collideable;
class c_base_entity;
class i_handle_entity;

enum trace_type {
    everything = 0,
    world_only,
    entities_only,
    everything_filter_props,
};

enum surface_flags {
    surface = ( 1 << 0 ),
    walkable = ( 1 << 1 ),
    buildable = ( 1 << 2 ),
    surfprop1 = ( 1 << 3 ),
    surfprop2 = ( 1 << 4 ),
};

struct cplane_t {
    vector_3d normal;
    float dist;
    uint8_t type;
    uint8_t sign_bits;
    PAD( 0x2 );
};

class c_trace_filter {
public:
    virtual bool should_hit_entity( void *ent, int mask ) = 0;
    virtual trace_type get_trace_type( ) const = 0;
};

class c_trace_filter_hitscan : public c_trace_filter {
public:
    virtual bool should_hit_entity( void *entity_handle, int contents_mask );
    virtual trace_type get_trace_type( ) const;
    c_base_entity *player = nullptr;
};

class c_trace_filter_skip_two_entities : public c_trace_filter {
public:
    void *skip1;
    void *skip2;

    c_trace_filter_skip_two_entities( ) {
        skip2 = skip1 = nullptr;
    }

    c_trace_filter_skip_two_entities( void *ent1, void *ent2 ) {
        skip1 = ent1;
        skip2 = ent2;
    }

    bool should_hit_entity( void *entity_handle, int contents_mask ) override {
        return !( entity_handle == skip1 || entity_handle == skip2 );
    }

    trace_type get_trace_type( ) const override {
        return trace_type::everything;
    }
};

class c_trace_filter_hit_all : public c_trace_filter {
public:
    bool should_hit_entity( void *entity_handle, int contents_mask );
    virtual trace_type get_trace_type( ) const;

    c_base_entity *skip = nullptr;
};

class c_trace_filter_world_and_props : public c_trace_filter {
public:
    bool should_hit_entity( void *entity_handle, int contents_mask ) {
        return false;
    }

    virtual trace_type get_trace_type( ) const {
        return trace_type::everything;
    }
};

class c_base_trace {
public:
    bool is_disp_surface( void ) { return ( ( flags & surface_flags::surface ) != 0 ); }
    bool is_disp_surface_walkable( void ) { return ( ( flags & surface_flags::walkable ) != 0 ); }
    bool is_disp_surface_buildable( void ) { return ( ( flags & surface_flags::buildable ) != 0 ); }
    bool is_disp_surface_surf_prop1( void ) { return ( ( flags & surface_flags::surfprop1 ) != 0 ); }
    bool is_disp_surface_surf_prop2( void ) { return ( ( flags & surface_flags::surfprop2 ) != 0 ); }

public:
    vector_3d start_pos = vector_3d( );
    vector_3d end_pos = vector_3d( );
    cplane_t plane = { };
    float fraction = 0.0f;
    uint32_t contents;
    uint16_t flags;
    bool all_solid;
    bool start_solid;

    c_base_trace( ) {
    }

private:
    c_base_trace( const c_base_trace &other );
};

struct csurface_t {
    const char *name;
    short surface_props;
    unsigned short flags;
};

class c_game_trace : public c_base_trace {
public:
    bool did_hit_world( ) const;

    bool did_hit_non_world_entity( ) const {
        return entity != nullptr && !did_hit_world( );
    }

    bool did_hit( ) const {
        return ( fraction < 1 || all_solid || start_solid );
    }

    float fraction_left_solid;
    csurface_t surface;
    int hit_group;
    short physics_bone;
    uint16_t world_surface_index;
    c_base_entity *entity;
    int hitbox;

    c_game_trace( ) {}
    c_game_trace( const c_game_trace &othr );
};

typedef bool ( *should_hit_func_t )( i_handle_entity *pHandleEntity, int contentsMask );

enum collision_group_t : int {
    COLLISION_GROUP_NONE = 0,
    COLLISION_GROUP_DEBRIS,
    COLLISION_GROUP_DEBRIS_TRIGGER,
    COLLISION_GROUP_INTERACTIVE_DEBRIS,
    COLLISION_GROUP_INTERACTIVE,
    COLLISION_GROUP_PLAYER,
    COLLISION_GROUP_BREAKABLE_GLASS,
    COLLISION_GROUP_VEHICLE,
    COLLISION_GROUP_PLAYER_MOVEMENT,
    COLLISION_GROUP_NPC,
    COLLISION_GROUP_IN_VEHICLE,
    COLLISION_GROUP_WEAPON,
    COLLISION_GROUP_VEHICLE_CLIP,
    COLLISION_GROUP_PROJECTILE,
    COLLISION_GROUP_DOOR_BLOCKER,
    COLLISION_GROUP_PASSABLE_DOOR,
    COLLISION_GROUP_DISSOLVING,
    COLLISION_GROUP_PUSHAWAY,
    COLLISION_GROUP_NPC_ACTOR,
    COLLISION_GROUP_NPC_SCRIPTED,
    COLLISION_GROUP_PZ_CLIP,
    COLLISION_GROUP_DEBRIS_BLOCK_PROJECTILE,
    LAST_SHARED_COLLISION_GROUP
};

class c_trace_filter_simple {
public:
    c_trace_filter_simple( ){ };

    c_trace_filter_simple( c_base_entity *pass_ent, int collision_group, should_hit_func_t extra_should_check_hit_fn = NULL ) {
        this->extra_should_check_hit_fn = extra_should_check_hit_fn;
        this->vftable = g_addresses.trace_filter_simple_vmt.get< void * >( );
        this->pass_ent = pass_ent;
        this->collision_group = collision_group;
    }

    bool should_hit_entity( c_base_entity *entity, int contents_mask ) {
        const auto real_vmt = *reinterpret_cast< void ** >( this->vftable );

        return utils::get_method< bool( __thiscall * )( void *, c_base_entity *, int ) >( real_vmt, 0 )( real_vmt, entity, contents_mask );
    }

    const c_base_entity *get_pass_entity( void ) { return pass_ent; }
    int get_collision_group( void ) const { return collision_group; }

private:
    void *vftable;
    c_base_entity *pass_ent;
    int collision_group;
    should_hit_func_t extra_should_check_hit_fn;
};

class c_trace_filter_entity : public c_trace_filter_simple {
public:
    c_trace_filter_entity( c_base_entity *pEntity, int nCollisionGroup );

    trace_type get_trace_type( ) const {
        return trace_type::entities_only;
    }

private:
    c_base_entity *root_parent;
    c_base_entity *entity;
    bool check_hash;
};

class c_engine_trace {
public:
    virtual int get_point_contents( const vector_3d &origin, int mask, c_base_entity **ent = nullptr ) = 0;
    virtual int get_point_contents_world_only( const vector_3d &origin, int mask ) = 0;
    virtual void *pad002( ) = 0;
    virtual void clip_ray_to_entity( const ray_t &ray, uint32_t mask, c_base_entity *entity, c_game_trace *trace ) = 0;
    virtual void clip_ray_to_collideable( const ray_t &ray, unsigned int mask, void *collide, c_game_trace *trace ) = 0;
    virtual void trace_ray( const ray_t &ray, unsigned int mask, void *filter, c_game_trace *trace ) = 0;
};