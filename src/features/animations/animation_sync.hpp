#pragma once
#include <globals.hpp>
#include <utils/circular_buffer.hpp>

enum class resolve_mode : int {
    none = -1,
    moving,
    standing,
    lby_update,
    adjust_stop,
    freestand,
    brute,
    air,
    air_brute
};

enum class velocity_detail : int {
    none = -1,
    zero,
    running,
    perfect,
    constant,
    accelerating
};

struct backup_record {
public:
    matrix_3x4 m_bones[ 128 ];
    int m_bone_count;
    float m_sim_time;
    int m_flags;
    vector_3d m_origin, m_abs_origin;
    vector_3d m_mins;
    vector_3d m_maxs;
    vector_3d m_abs_ang;

public:
    __forceinline void store( c_cs_player *player ) {
        // get bone cache ptr.
        auto cache = player->bone_cache( );

        if ( !cache )
            return;

        // store bone data.
        std::memcpy( m_bones, cache, sizeof( matrix_3x4 ) * 128 );

        //m_bone_count = player->bone_count( );
        m_origin = player->origin( );
        m_mins = player->collideable( )->mins( );
        m_sim_time = player->simtime( );
        m_flags = player->flags( );
        m_maxs = player->collideable( )->maxs( );
        m_abs_origin = player->get_abs_origin( );
        m_abs_ang = player->get_abs_angles( );
    }

    __forceinline void restore( c_cs_player *player ) {
        // get bone cache ptr.
        auto cache = player->bone_cache( );

        if ( !cache )
            return;

        std::memcpy( cache, m_bones, sizeof( matrix_3x4 ) * m_bone_count );

        player->bone_count( ) = m_bone_count;
        player->origin( ) = m_origin;
        player->set_collision_bounds( m_mins, m_maxs );
        player->set_abs_angles( m_abs_ang );
        player->set_abs_origin( m_origin );
    }
};

struct lag_record {
public:
    lag_record( ) = default;

    lag_record( c_cs_player *player ) {
        this->reset( player );
    }

    int flags;
    vector_3d origin;
    bool on_ground;
    bool real_on_ground;
    float lower_body_yaw;
    vector_3d angles;
    vector_3d abs_origin;
    vector_3d velocity;
    velocity_detail velocity_detail;
    int arrival_tick;
    vector_3d anim_velocity;
    vector_3d abs_angles;
    matrix_3x4 renderable_to_world_transform;
    float anim_speed;
    vector_3d mins;
    vector_3d maxs;
    bool fake_walk;
    c_cs_player *player;
    vector_3d eye_angles;
    bool moved;
    resolve_mode mode;
    float sim_time;
    float anim_time;
    bool forward_track;
    int index;
    float old_sim_time;
    bool break_lc;
    bool dormant;
    float duck_amount;
    int choked;
    bool is_exploit;
    bool shot;
    c_csgo_player_animstate anim_state;
    std::array< c_animation_layer, 15 > layer_records = { };
    std::array< float, 24 > pose_parameters = { };
    std::array< matrix_3x4, 128 > bones = { };
    bool record_filled;
    bool has_velocity;
    const model_t *model;
    int tick;

    void reset( c_cs_player *player );
    bool is_valid( );
    void cache( );

    void apply( ) {
    }
};

struct aim_player {
    aim_player( ) = default;

    aim_player( c_cs_player *entity, float fov, float distance ) {
        this->entity = entity;
        this->fov = fov;
        this->distance = distance;
        this->delay_shot = false;
    }

    c_cs_player *entity;
    float fov;
    float distance;
    bool delay_shot;
};

class animation_sync {
public:
    void on_post_frame_stage_notify( client_frame_stage stage );
    void extrapolate( lag_record *record, vector_3d &origin, vector_3d &velocity, int &flags, bool on_ground );
    void apply_animations( );
    void on_pre_frame_stage_notify( client_frame_stage stage );
    void on_net_update_end( );
    bool fix_velocity( c_cs_player *ent, vector_3d &vel, const std::array< c_animation_layer, 15 > &animlayers, const vector_3d &origin );
    void clear_data( int index );
    void generate_shoot_position( );
    bool should_predict_lag( aim_player &target, lag_record *record, lag_record *previous );
    bool get_lagcomp_bones( c_cs_player *player, std::array< matrix_3x4, 128 > &out );
    bool build_bones( c_cs_player *player, matrix_3x4 *out, std::array< float, 24 > &poses, float curtime );
    void update_land( c_cs_player *player, lag_record *record, lag_record *last_record );
    void update_velocity( c_cs_player *player, lag_record *record, lag_record *previous );
    void update_lby_timer( c_csgo_player_animstate *state, c_user_cmd *user_cmd );
    void update_local_animations( c_user_cmd *user_cmd );
    void maintain_local_animations( );
    void update_player_animation( c_cs_player *player, lag_record &record, lag_record *previous, bool update = true );

    velocity_detail fix_velocity( c_animation_layer *animlayers, lag_record *previous, c_cs_player *player );

    struct animation_info {
        util::circular_buffer< lag_record > anim_records;
        std::deque< lag_record * > lag_records;
        float spawn_time;
        int index;
        int shots;
        int missed_shots;
        c_cs_player *player;
        float body_update_time;
        float body;
        float last_lby;
        bool lby_updated;
        bool valid{ };
        lag_record walk_record;
        player_info_t player_info;

        struct freestand_record_t {
            void reset( ) {
                this->left_damage = this->right_damage = 0.0f;
                this->left_fraction = this->right_fraction = 0.0f;
            }

            freestand_record_t( ) {
                reset( );
            }

            float left_damage, right_damage;
            float left_fraction, right_fraction;
        } anti_freestand_record;

        struct resolve_record_t {
            void reset( ) {
                this->predicted_lby_flick = false;
                this->lby_flick = false;
            }

            resolve_record_t( ) {
                reset( );
            }

            bool predicted_lby_flick;
            bool lby_flick;
        } resolve_record;

        void reset( c_cs_player *entity ) {
            index = entity->index( );
            player = entity;
            spawn_time = entity->spawn_time( );
            anim_records.clear( );
            shots = 0;
            body_update_time = 0.f;
            missed_shots = 0;

            walk_record.reset( entity );
            resolve_record.reset( );
            anti_freestand_record.reset( );
        }
    };

    std::array< animation_info, 64 > player_log;
    std::array< std::array< matrix_3x4, 128 >, 64 > animated_bones;
    std::array< vector_3d, 64 > animated_origin;
    float lower_body_realign_timer;
    float foot_yaw;
    float last_angle;
    vector_3d abs_rotation;
    float simtime;
    vector_3d radar_angle;
    bool updating_lby;
    float body;
    bool on_ground;
    std::array< float, 24 > pose_parameters;
    std::array< c_animation_layer, 13 > animation_layers;
    std::array< c_animation_layer, 13 > queued_animation_layers;
    std::array< c_animation_layer, 13 > playback_animation_layers;

private:
};

inline animation_sync g_animations = { };