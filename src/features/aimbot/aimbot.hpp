#pragma once
#include <features/animations/animation_sync.hpp>
#include <globals.hpp>
#include <features/penetration/penetration.hpp>

struct aim_player {
    aim_player( c_cs_player *entity, float fov, float distance, int hitbox, lag_record *record, vector_3d pos, vector_3d center ) {
        this->entity = entity;
        this->fov = fov;
        this->distance = distance;
        this->record = record;
        this->hitbox = hitbox;
        this->positon = pos;
        this->center = center;
    }

    c_cs_player *entity;
    float fov;
    float distance;
    lag_record *record;
    int hitbox;
    vector_3d positon;
    vector_3d center;
    c_fire_bullet_data bullet_data;
};

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
    vector_3d pos;
    bool center;
    lag_record *record;
    c_fire_bullet_data bullet_data;
    int hb;
};

struct thread_args {
    aim_point *point;
    int hb;
};

class aimbot {
private:
    void search_player( c_cs_player *player );
    void search_targets( );
    void generate_points_for_hitbox( c_cs_player *player, lag_record *record, int side, std::vector< std::pair< vector_3d, bool > > &points, mstudiobbox_t *hitbox, mstudiohitboxset_t *set, int idx, float scale );
    void generate_points( c_cs_player *player, lag_record* record );
    bool scan_target( c_cs_player *player, lag_record *record, aim_player &target );
    void adjust_speed( c_user_cmd *ucmd );

public:
    std::deque< aim_point > aim_points = { };
    std::deque< int > hitboxes = { };
    std::deque< aim_player > targets = { };
    target_info best = { };
    backup_record m_backup[ 64 ];

    void reset( );
    bool hitchance( c_cs_player *player, const vector_3d &angle, lag_record *record );
    void on_create_move( c_user_cmd *cmd );
    bool get_hitbox_position( c_cs_player *player, matrix_3x4 *bones, int hitbox, vector_3d &position );
};

inline aimbot g_aimbot = { };