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

struct thread_args {
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
    void search_player( c_cs_player *player );
    void search_targets( );
    void generate_points_for_hitbox( c_cs_player *player, lag_record *record, int side, std::vector< std::pair< vector_3d, bool > > &points, mstudiobbox_t *hitbox, mstudiohitboxset_t *set, int idx, float scale );
    void generate_points( c_cs_player *player, lag_record* record );
    bool scan_target( c_cs_player *player, lag_record *record, aim_player &target );
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
    bool can_hit_player( c_cs_player *player, vector_3d start, vector_3d end, lag_record *record, matrix_3x4 *matrix );
    bool can_hit_player( c_cs_player *player, vector_3d start, vector_3d end, lag_record *record, int hitbox, matrix_3x4 *matrix );

    bool should_hit( c_cs_player *player, const vector_3d &angle, lag_record *record );
    void on_create_move( c_user_cmd *cmd );
    bool get_hitbox_position( c_cs_player *player, matrix_3x4 *bones, int hitbox, vector_3d &position );
};

inline ragebot g_ragebot = { };