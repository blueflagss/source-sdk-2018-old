#pragma once
#include <globals.hpp>

class skin_changer {
public:
    void init( );
    void apply_knifes( );
    void force_full_update( );
    void do_sequence_remapping( c_recv_proxy_data *data, c_base_view_model *entity );
    void on_post_data_update_start( );

    enum ItemDefinitionIndex : int {
        WEAPON_DEAGLE = 1,
        WEAPON_ELITE = 2,
        WEAPON_FIVESEVEN = 3,
        WEAPON_GLOCK = 4,
        WEAPON_AK47 = 7,
        WEAPON_AUG = 8,
        WEAPON_AWP = 9,
        WEAPON_FAMAS = 10,
        WEAPON_G3SG1 = 11,
        WEAPON_GALILAR = 13,
        WEAPON_M249 = 14,
        WEAPON_M4A1 = 16,
        WEAPON_MAC10 = 17,
        WEAPON_P90 = 19,
        WEAPON_MP5_SD = 23,
        WEAPON_UMP45 = 24,
        WEAPON_XM1014 = 25,
        WEAPON_BIZON = 26,
        WEAPON_MAG7 = 27,
        WEAPON_NEGEV = 28,
        WEAPON_SAWEDOFF = 29,
        WEAPON_TEC9 = 30,
        WEAPON_TASER = 31,
        WEAPON_HKP2000 = 32,
        WEAPON_MP7 = 33,
        WEAPON_MP9 = 34,
        WEAPON_NOVA = 35,
        WEAPON_P250 = 36,
        WEAPON_SCAR20 = 38,
        WEAPON_SG556 = 39,
        WEAPON_SSG08 = 40,
        WEAPON_KNIFE = 42,
        WEAPON_FLASHBANG = 43,
        WEAPON_HEGRENADE = 44,
        WEAPON_SMOKEGRENADE = 45,
        WEAPON_MOLOTOV = 46,
        WEAPON_DECOY = 47,
        WEAPON_INCGRENADE = 48,
        WEAPON_C4 = 49,
        WEAPON_KNIFE_T = 59,
        WEAPON_M4A1_SILENCER = 60,
        WEAPON_USP_SILENCER = 61,
        WEAPON_CZ75A = 63,
        WEAPON_REVOLVER = 64,
        WEAPON_KNIFE_BAYONET = 500,
        WEAPON_KNIFE_CSS = 503,
        WEAPON_KNIFE_FLIP = 505,
        WEAPON_KNIFE_GUT = 506,
        WEAPON_KNIFE_KARAMBIT = 507,
        WEAPON_KNIFE_M9_BAYONET = 508,
        WEAPON_KNIFE_TACTICAL = 509,
        WEAPON_KNIFE_FALCHION = 512,
        WEAPON_KNIFE_SURVIVAL_BOWIE = 514,
        WEAPON_KNIFE_BUTTERFLY = 515,
        WEAPON_KNIFE_PUSH = 516,
        WEAPON_KNIFE_CORD = 517,
        WEAPON_KNIFE_CANIS = 518,
        WEAPON_KNIFE_URSUS = 519,
        WEAPON_KNIFE_GYPSY_JACKKNIFE = 520,
        WEAPON_KNIFE_OUTDOOR = 521,
        WEAPON_KNIFE_STILETTO = 522,
        WEAPON_KNIFE_WIDOWMAKER = 523,
        WEAPON_KNIFE_SKELETON = 525,
        GLOVE_STUDDED_BLOODHOUND = 5027,
        GLOVE_T_SIDE = 5028,
        GLOVE_CT_SIDE = 5029,
        GLOVE_SPORTY = 5030,
        GLOVE_SLICK = 5031,
        GLOVE_LEATHER_WRAP = 5032,
        GLOVE_MOTORCYCLE = 5033,
        GLOVE_SPECIALIST = 5034,
        GLOVE_HYDRA = 5035
    };

    struct weapon_info {
        constexpr weapon_info( const char *model, const char *icon = nullptr ) : model( model ),
                                                                                 icon( icon ) {}

        const char *model;
        const char *icon;
    };

    struct weapon_name {
        constexpr weapon_name( const int definition_index, const char *name ) : definition_index( definition_index ),
                                                                                name( name ) {}

        int definition_index = 0;
        const char *name = nullptr;
    };

    struct quality_name {
        constexpr quality_name( const int index, const char *name ) : index( index ),
                                                                      name( name ) {}

        int index = 0;
        const char *name = nullptr;
    };

    struct paint_kit {
        int id;
        std::string name;

        auto operator<( const paint_kit &other ) const -> bool {
            return name < other.name;
        }
    };

    std::vector< weapon_name > knife_names = {
            { 0, "Default" },
            { WEAPON_KNIFE_BAYONET, "Bayonet" },
            { WEAPON_KNIFE_CSS, "Classic Knife" },
            { WEAPON_KNIFE_FLIP, "Flip Knife" },
            { WEAPON_KNIFE_GUT, "Gut Knife" },
            { WEAPON_KNIFE_KARAMBIT, "Karambit" },
            { WEAPON_KNIFE_M9_BAYONET, "M9 Bayonet" },
            { WEAPON_KNIFE_TACTICAL, "Huntsman Knife" },
            { WEAPON_KNIFE_FALCHION, "Falchion Knife" },
            { WEAPON_KNIFE_SURVIVAL_BOWIE, "Bowie Knife" },
            { WEAPON_KNIFE_BUTTERFLY, "Butterfly Knife" },
            { WEAPON_KNIFE_PUSH, "Shadow Daggers" },
            { WEAPON_KNIFE_CORD, "Paracord Knife" },
            { WEAPON_KNIFE_CANIS, "Survival Knife" },
            { WEAPON_KNIFE_URSUS, "Ursus Knife" },
            { WEAPON_KNIFE_GYPSY_JACKKNIFE, "Navaja Knife" },
            { WEAPON_KNIFE_OUTDOOR, "Nomad Knife" },
            { WEAPON_KNIFE_STILETTO, "Stiletto Knife" },
            { WEAPON_KNIFE_WIDOWMAKER, "Talon Knife" },
            { WEAPON_KNIFE_SKELETON, "Skeleton Knife" } 
    };

    std::vector< weapon_name > glove_names = {
            { 0, "Default" },
            { GLOVE_STUDDED_BLOODHOUND, "Bloodhound" },
            { GLOVE_T_SIDE, "Default (Terrorists)" },
            { GLOVE_CT_SIDE, "Default (Counter-Terrorists)" },
            { GLOVE_SPORTY, "Sporty" },
            { GLOVE_SLICK, "Slick" },
            { GLOVE_LEATHER_WRAP, "Handwrap" },
            { GLOVE_MOTORCYCLE, "Motorcycle" },
            { GLOVE_SPECIALIST, "Specialist" },
            { GLOVE_HYDRA, "Hydra" } 
    };

    std::vector< weapon_name > weapon_names =
            {
                    { WEAPON_KNIFE, "Knife" },
                    { GLOVE_T_SIDE, "Glove" },
                    { 7, "AK-47" },
                    { 8, "AUG" },
                    { 9, "AWP" },
                    { 63, "CZ75 Auto" },
                    { 1, "Desert Eagle" },
                    { 2, "Dual Berettas" },
                    { 10, "FAMAS" },
                    { 3, "Five-SeveN" },
                    { 11, "G3SG1" },
                    { 13, "Galil AR" },
                    { 4, "Glock-18" },
                    { 14, "M249" },
                    { 60, "M4A1-S" },
                    { 16, "M4A4" },
                    { 17, "MAC-10" },
                    { 27, "MAG-7" },
                    { 23, "MP5-SD" },
                    { 33, "MP7" },
                    { 34, "MP9" },
                    { 28, "Negev" },
                    { 35, "Nova" },
                    { 32, "P2000" },
                    { 36, "P250" },
                    { 19, "P90" },
                    { 26, "PP-Bizon" },
                    { 64, "R8 Revolver" },
                    { 29, "Sawed-Off" },
                    { 38, "SCAR-20" },
                    { 40, "SSG 08" },
                    { 39, "SG 553" },
                    { 30, "Tec-9" },
                    { 24, "UMP-45" },
                    { 61, "USP-S" },
                    { 25, "XM1014" },
    };

    std::vector< quality_name > quality_names = {
            { 0, "Default" },
            { 1, "Genuine" },
            { 2, "Vintage" },
            { 3, "Unusual" },
            { 5, "Community" },
            { 6, "Developer" },
            { 7, "Self-Made" },
            { 8, "Customized" },
            { 9, "Strange" },
            { 10, "Completed" },
            { 12, "Tournament" } 
    };

    std::vector< paint_kit > skin_kits;
    std::vector< paint_kit > glove_kits;
};

inline skin_changer g_skin_changer;