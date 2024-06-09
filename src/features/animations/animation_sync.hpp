#pragma once
#include <globals.hpp>
#include <utils/circular_buffer.hpp>

enum class resolve_mode : int {
    none = -1,
    walk,
    stand,
    flick,
    air
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
    vector_3d angles;
    vector_3d abs_origin;
    vector_3d velocity;
    vector_3d anim_velocity;
    vector_3d abs_angles;
    float anim_speed;
    vector_3d mins;
    vector_3d maxs;
    bool fake_walk;
    c_cs_player *player;
    vector_3d eye_angles;
    resolve_mode mode;
    float sim_time;
    float anim_time;
    int index;
    float old_sim_time;
    bool break_lc;
    bool dormant;
    int choked;
    c_csgo_player_animstate *anim_state;
    std::array< c_animation_layer, 15 > layer_records = { };
    std::array< float, 24 > pose_parameters = { };
    std::array< matrix_3x4, 128 > bones = { };
    bool record_filled;
    bool has_velocity;
    const model_t *model;

    void reset( c_cs_player *player );
    bool is_valid( );
    void cache( );

    void apply( ) {
    }
};

class animation_sync {
public:
    void should_interpolate( c_cs_player *player, bool state );
    void on_net_update_postdataupdate_start( );
    void on_pre_frame_render_start( );
    void on_post_frame_render_start( );
    void on_net_update_end( );
    bool fix_velocity( c_cs_player *ent, vector_3d &vel, const std::array< c_animation_layer, 15 > &animlayers, const vector_3d &origin );
    void clear_data( int index );
    bool get_lagcomp_bones( c_cs_player *player, std::array< matrix_3x4, 128 > &out );
    bool build_bones( c_cs_player *player, matrix_3x4 *out, float curtime );
    void update_land( c_cs_player *player, lag_record *record, lag_record *last_record );
    void update_velocity( c_cs_player *player, lag_record *record, lag_record *previous );
    void update_player_animation( c_cs_player *player, lag_record &record, lag_record *previous );

    struct animation_info {
        util::circular_buffer< lag_record > anim_records;
        std::deque< lag_record * > lag_records;
        float spawn_time;
        int index;
        c_cs_player *player;

        void reset( c_cs_player *entity ) {
            index = entity->index( );
            player = entity;
            spawn_time = entity->spawn_time( );
            anim_records.clear( );
        }
    };

    std::array< animation_info, 64 > lag_info;
    std::array< std::array< matrix_3x4, 128 >, 64 > animated_bones;
    std::array< vector_3d, 64 > animated_origin;

    inline std::deque< lag_record * > get_lagcomp_records( c_cs_player *player ) {
        if ( !player || !player->index( ) || !player->is_player( ) )
            return { };

        auto &info = lag_info[ player->index( ) ];

        if ( player->index( ) > g_interfaces.global_vars->max_clients || info.anim_records.empty( ) )
            return { };

        std::deque< lag_record * > records{ };


        return records;
    }

private:
};

inline animation_sync g_animations = { };