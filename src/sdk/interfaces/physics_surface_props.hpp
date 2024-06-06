#pragma once

enum surf : uint32_t {
    surf_light = 0x0001,
    surf_sky2d = 0x0002,
    surf_sky = 0x0004,
    surf_warp = 0x0008,
    surf_trans = 0x0010,
    surf_noportal = 0x0020,
    surf_trigger = 0x0040,
    surf_nodraw = 0x0080,
    surf_hint = 0x0100,
    surf_skip = 0x0200,
    surf_nolight = 0x0400,
    surf_bumplight = 0x0800,
    surf_noshadows = 0x1000,
    surf_nodecals = 0x2000,
    surf_nopaint = surf_nodecals,
    surf_nochop = 0x4000,
    surf_hitbox = 0x8000
};

struct surfacedata_t;

struct surfacephysicsparams_t {
    PAD( 20 );
};

struct surfaceaudioparams_t {
    PAD( 36 );
};

struct surfacesoundnames_t {
    PAD( 24 );
};

struct surfacegameprops_t {
    PAD( 8 );
    float penetration_modifier;
    float damage_modifier;
    unsigned short material;
    unsigned char climbable;
    PAD( 4 );
};

struct surfacedata_t {
    surfacephysicsparams_t physics;
    surfaceaudioparams_t audio;
    surfacesoundnames_t sounds;
    surfacegameprops_t game;
    PAD( 48 );
};

class c_physics_props {
public:
    surfacedata_t *get_surface_data( int idx ) {
        return utils::get_method< surfacedata_t *( __thiscall * ) ( void *, int ) >( this, 5 )( this, idx );
    }
};