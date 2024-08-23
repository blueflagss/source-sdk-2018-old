#pragma once
#include <features/animations/animation_sync.hpp>
#include <features/grenade/grenade_warning.hpp>
#include <globals.hpp>

inline std::map< int, const char * > csgo_hud_icons = 
{
        { weapons::deagle, ( const char * )u8"\uE001" },
        { weapons::elite, ( const char * )u8"\uE002" },
        { weapons::fiveseven, ( const char * )u8"\uE003" },
        { weapons::glock, ( const char * )u8"\uE004" },
        { weapons::ak47, ( const char * )u8"\uE007" },
        { weapons::aug, ( const char * )u8"\uE008" },
        { weapons::awp, ( const char * )u8"\uE009" },
        { weapons::famas, ( const char * )u8"\uE00A" },
        { weapons::g3sg1, ( const char * )u8"\uE00B" },
        { weapons::galil, ( const char * )u8"\uE00D" },
        { weapons::m249, ( const char * )u8"\uE03C" },
        { weapons::m4a4, ( const char * )u8"\uE00E" },
        { weapons::mac10, ( const char * )u8"\uE011" },
        { weapons::p90, ( const char * )u8"\uE024" },
        { weapons::ump45, ( const char * )u8"\uE018" },
        { weapons::xm1014, ( const char * )u8"\uE019" },
        { weapons::bizon, ( const char * )u8"\uE01A" },
        { weapons::mag7, ( const char * )u8"\uE027" },
        { weapons::negev, ( const char * )u8"\uE01C" },
        { weapons::sawedoff, ( const char * )u8"\uE01D" },
        { weapons::tec9, ( const char * )u8"\uE01E" },
        { weapons::zeus, ( const char * )u8"\uE01F" },
        { weapons::p2000, ( const char * )u8"\uE020" },
        { weapons::mp7, ( const char * )u8"\uE021" },
        { weapons::mp9, ( const char * )u8"\uE022" },
        { weapons::nova, ( const char * )u8"\uE023" },
        { weapons::p250, ( const char * )u8"\uE013" },
        { weapons::scar20, ( const char * )u8"\uE026" },
        { weapons::ssg556, ( const char * )u8"\uE027" },
        { weapons::ssg08, ( const char * )u8"\uE028" },
        { weapons::knife_t, ( const char * )u8"\uE02A" },
        { weapons::flashbang, ( const char * )u8"\uE02B" },
        { weapons::hegrenade, ( const char * )u8"\uE02C" },
        { weapons::smoke, ( const char * )u8"\uE02D" },
        { weapons::molotov, ( const char * )u8"\uE02E" },
        { weapons::decoy, ( const char * )u8"\uE02F" },
        { weapons::firebomb, ( const char * )u8"\uE030" },
        { weapons::c4, ( const char * )u8"\uE031" },
        { weapons::knife_ct, ( const char * )u8"\uE03B" },
        { weapons::m4a1s, ( const char * )u8"\uE010" },
        { weapons::usps, ( const char * )u8"\uE03D" },
        { weapons::cz75a, ( const char * )u8"\uE03F" },
        { weapons::revolver, ( const char * )u8"\uE040" },
        { weapons::knife_bayonet, ( const char * )u8"\uE1F4" },
        { weapons::knife_flip, ( const char * )u8"\uE1F9" },
        { weapons::knife_gut, ( const char * )u8"\uE1FA" },
        { weapons::knife_karambit, ( const char * )u8"\uE1FB" },
        { weapons::knife_m9_bayonet, ( const char * )u8"\uE1FC" },
        { weapons::knife_huntsman, ( const char * )u8"\uE1FD" },
        { weapons::knife_falchion, ( const char * )u8"\uE200" },
        { weapons::knife_bowie, ( const char * )u8"\uE202" },
        { weapons::knife_butterfly, ( const char * )u8"\uE203" },
        { weapons::knife_shadow_daggers, ( const char * )u8"\uE204" } 
};

class visuals {
public:
    void render( );
    void render_grenade_trail( );
    void indicators( );
    void hitmarker( );
    void update_configuration( c_base_entity *entity );
    void render_weapon( c_cs_weapon_base *weapon );
    void manual_arrows( );
    void render_grenade( c_base_cs_grenade *grenade );
    void scope_lines( );
    void autowall_crosshair( );
    void on_create_move( );
    void world_modulation( );
    void render_bar( c_cs_player *player, const box_t &box, const float &value, const float &progress, color col, bool show = false );
   
    float hitmarker_fraction = 0.0f;

private:
    int distance_offset = 0;
    std::deque< float > opacity_array;

    class config_info {
    public:
        __forceinline config_info( ) {
            bottom_bar_offset = 0;
            weapon_offset = 0;
        }

        color name_color;
        color box_color;
        color weapon_name_color;
        color offscreen_color;
        int bottom_bar_offset;
        int weapon_offset;
        bool dormant;
    };
    std::array< config_info, 64 > esp_config;

    class penetration_data {
    public:
        __forceinline penetration_data( ) {
            damage = 0;
        }

        float damage;
    } pen_data;

private:
    void render_player( c_cs_player *player );
    void render_offscreen( c_cs_player *player ) const;
    void render_skeleton( c_cs_player *player, matrix_3x4 *bones, color skeleton_color ) const;

    std::map< uint32_t, grenade_warning > predicted_grenades;
};

inline visuals g_visuals = { };