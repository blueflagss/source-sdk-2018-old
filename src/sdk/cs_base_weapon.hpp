#pragma once
#include <globals.hpp>
#include <utils/padding.hpp>
#include "cs_player.hpp"

enum weapons : int {
    deagle = 1,
    elite = 2,
    fiveseven = 3,
    glock = 4,
    ak47 = 7,
    aug = 8,
    awp = 9,
    famas = 10,
    g3sg1 = 11,
    galil = 13,
    m249 = 14,
    m4a4 = 16,
    mac10 = 17,
    p90 = 19,
    ump45 = 24,
    xm1014 = 25,
    bizon = 26,
    mag7 = 27,
    negev = 28,
    sawedoff = 29,
    tec9 = 30,
    zeus = 31,
    p2000 = 32,
    mp7 = 33,
    mp9 = 34,
    nova = 35,
    p250 = 36,
    scar20 = 38,
    ssg556 = 39,
    ssg08 = 40,
    knife_t = 42,
    flashbang = 43,
    hegrenade = 44,
    smoke = 45,
    molotov = 46,
    decoy = 47,
    firebomb = 48,
    c4 = 49,
    musickit = 58,
    knife_ct = 59,
    m4a1s = 60,
    usps = 61,
    tradeupcontract = 62,
    cz75a = 63,
    revolver = 64,
    knife_bayonet = 500,
    knife_flip = 505,
    knife_gut = 506,
    knife_karambit = 507,
    knife_m9_bayonet = 508,
    knife_huntsman = 509,
    knife_falchion = 512,
    knife_bowie = 514,
    knife_butterfly = 515,
    knife_shadow_daggers = 516,
};

enum weapon_type : int {
    WEAPONTYPE_UNKNOWN = -1,
    WEAPONTYPE_KNIFE,
    WEAPONTYPE_PISTOL,
    WEAPONTYPE_SUBMACHINEGUN,
    WEAPONTYPE_RIFLE,
    WEAPONTYPE_SHOTGUN,
    WEAPONTYPE_SNIPER_RIFLE,
    WEAPONTYPE_MACHINEGUN,
    WEAPONTYPE_C4,
    WEAPONTYPE_TASER,
    WEAPONTYPE_GRENADE,
    WEAPONTYPE_HEALTHSHOT = 11
};

class c_cs_weapon_info {
public:
    PAD( 4 );                    // 0x0000
    const char *weapon_name;     // 0x0004
    PAD( 12 );                   // 0x0008
    int max_clip;                // 0x0014
    int max_clip_alt;            // 0x0018
    int default_clip;            // 0x001C
    int default_clip_alt;        // 0x0020
    PAD( 100 );                  // 0x0024
    const char *unk1;            // 0x0088
    const char *unk2;            // 0x008C
    PAD( 56 );                   // 0x0090
    int weapon_type;             // 0x00C8
    PAD( 8 );                    // 0x00CC
    const char *weapon_name_alt; // 0x00D4
    PAD( 20 );                   // 0x00D8
    int damage;                  // 0x00EC
    float armor_ratio;           // 0x00F0
    int bullets;                 // 0x00F4
    float penetration;           // 0x00F8
    PAD( 8 );                    // 0x00FC
    float range;                 // 0x0104
    float range_modifier;        // 0x0108
    float throw_velocity;        // 0x010C
    PAD( 28 );                   // 0x0110
    float max_speed;             // 0x012C
    float max_speed_alt;         // 0x0130
    float spread;                // 0x0134
    float spread_alt;            // 0x0138
    float inaccuracy_crouch;     // 0x013C
    float inaccuracy_crouch_alt; // 0x0140
    float inaccuracy_stand;      // 0x0144
    float inaccuracy_stand_alt;  // 0x0148
}; //Size: 0x01A0

class c_econ_item_definition {
public:
    virtual const char *index( ) = 0;
    virtual const char *prefab_name( ) = 0;
    virtual const char *item_base_name( ) = 0;
};

class c_cs_weapon_base : public c_cs_player {
public:
    DATAMAP( next_burst_shot, float, this->get_pred_desc_map( ), "m_fNextBurstShot" );
    NETVAR( last_shot_time, float, "DT_WeaponCSBase", "m_fLastShotTime" );
    NETVAR( weapon_world_model, uint32_t, "DT_WeaponCSBase", "m_hWeaponWorldModel" );
    NETVAR( recoil_index, float, "DT_WeaponNegev", "m_flRecoilIndex" );
    NETVAR( activity, int, "DT_WeaponCSBase", "m_Activity" );
    NETVAR( burst_shots_remaining, int, "DT_WeaponCSBase", "m_iBurstShotsRemaining" );
    NETVAR( item_definition_index, int, "DT_BaseAttributableItem", "m_iItemDefinitionIndex" );
    NETVAR( accuracy_penalty, float, "DT_WeaponCSBase", "fAccuracyPenalty" );
    NETVAR( next_primary_attack, float, "DT_WeaponCSBase", "m_flNextPrimaryAttack" );
    NETVAR( clip_1, int, "DT_BaseCombatWeapon", "m_iClip1" );
    NETVAR( clip_2, int, "DT_BaseCombatWeapon", "m_iClip2" );
    NETVAR( zoom_level, float, "DT_WeaponCSBaseGun", "m_zoomLevel" );
    NETVAR( primary_reserve_ammo_count, int, "DT_BaseCombatWeapon", "m_iPrimaryReserveAmmoCount" );

    float get_spread( );
    bool is_base_combat_weapon( );
    float get_inaccuracy( );
    vector_3d calculate_spread( int seed, bool revolver2 = false );
    vector_3d calculate_spread( int seed, float inaccuracy, float spread, bool revolver2 = false );
    c_cs_weapon_info *get_weapon_data( );
    wchar_t *get_name( );
    c_econ_item_definition *econ_item_view( c_cs_weapon_base *weapon );
    c_econ_item_definition *static_data( c_econ_item_definition *item );
    bool is_grenade( );
    void update_accuracy_penalty( );
    float get_lowest_accuracy( );
    bool scoped_weapon( );
};