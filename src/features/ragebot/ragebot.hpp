#pragma once
#include <features/animations/animation_sync.hpp>
#include <globals.hpp>
#include <features/penetration/penetration.hpp>

struct target_info {
    target_info( ) {
        hitbox = 0;
        target = nullptr;
        record = { };
        damage = 0.0f;
        best_point = vector_3d( 0.0f, 0.0f, 0.0f );
    }

    lag_record *record;
    float damage;
    c_cs_player *target;
    int hitbox;
    vector_3d best_point;
};

struct aim_point {
    aim_point( ) {
        pos = vector_3d{ };
        center = false;
        record = nullptr;
        bullet_data = c_fire_bullet_data{ };
        hb = -1;
    }

    vector_3d pos;
    bool center;
    lag_record *record;
    c_fire_bullet_data bullet_data;
    int hb;
};

struct hitscan_data {
    bool valid{ };
    bool done{ };
    int hb;
    lag_record *record;
    c_cs_player *target;
    std::vector< std::pair< vector_3d, bool > > points;
};

class hitbox_data {
public:
    int hitbox_id = 0;
    bool is_obb;
    vector_3d mins;
    vector_3d maxs;
    vector_3d start_scaled;
    float radius;
    mstudiobbox_t *hitbox;
    int bone;
    int hitgroup;
};

namespace hitscan_info
{
    inline target_info best = { };
}

class ragebot {
private:
    void search_targets( );
    bool scan_target( c_cs_player *player, lag_record *record, aim_player &target );
    static void hitscan_thread( hitscan_data *args );
    void adjust_speed( c_user_cmd *ucmd );
    bool get_hitbox_data( vector_3d start, hitbox_data *rtn, c_cs_player *player, int hitbox, matrix_3x4 *matrix );

public:
    c_fire_bullet_data bullet_data;
    bool found_point = false;
    std::deque< aim_player > targets = { };
    std::array< aim_player, 64 > players;
    backup_record m_backup[ 64 ];
    bool should_continue_thread;
    target_info best = { };

    std::vector< int > get_hitboxes( );
    void reset( );
    bool can_hit( c_cs_player *player, vector_3d start, vector_3d end, lag_record *record, matrix_3x4 *matrix );

    bool calculate_hitchance( c_cs_player *player, const int &hitbox, const vector_3d &angle, lag_record *record );
    void on_create_move( c_user_cmd *cmd );
    bool get_hitbox_position( c_cs_player *player, matrix_3x4 *bones, int hitbox, vector_3d &position );
};

inline ragebot g_ragebot = { };