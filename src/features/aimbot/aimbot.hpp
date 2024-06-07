#pragma once
#include <features/animations/anims.hpp>
#include <globals.hpp>

struct aim_player {
    aim_player( c_cs_player *entity, int index, int health, float distance, float fov, int hit_group, lag_record *record ) {
        this->entity = entity;
        this->index = index;
        this->fov = fov;
        this->health = health;
        this->distance = distance;
        this->record = record;
        this->hit_group = hit_group;
    }

    c_cs_player *entity;
    int index;
    float fov;
    int health;
    float distance;
    lag_record *record;
    int hit_group;
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

class aimbot {
private:
    void search_player( c_cs_player *player );
    void search_targets( );
    bool get_best_aim_position( aim_player &target, float &dmg, vector_3d &position, lag_record *record );
    bool scan_target( c_cs_player *player, lag_record *record, aim_player &target );

public:
    std::deque< vector_3d > points = { };
    std::deque< int > hitboxes = { };
    std::deque< aim_player > targets = { };
    target_info best = { };
    backup_record m_backup[ 64 ];

    void reset( );
    bool hitchance( c_cs_player *player, const vector_3d &angle, lag_record *record );
    void search_building( c_base_object *building );
    void on_create_move( c_user_cmd *cmd );
    bool setup_point_for_scan( c_cs_player *player, lag_record &record, int hit_group );
    bool get_hitbox_position( c_cs_player *player, matrix_3x4 *bones, int hitbox, vector_3d &position );
};

inline aimbot g_aimbot = { };