#pragma once
#include <globals.hpp>
#include <sdk/other/prediction_copy.hpp>

struct prediction_data {
    int command_sequence;

    void reset( ) {
        this->command_sequence = 0;
    }

    struct tickbase_t {
        int limit = { };
        int sent_commands = { };
        int choked_commands = { };
    } tickbase = { };
};

struct datamap_util {
    datamap_util( ) = default;

    datamap_util( c_base_entity *entity, const char* pre_action, const char* post_action ) {
        if ( !entity )
            return;

        const auto desc_map = entity->get_pred_desc_map( );

        if ( !desc_map )
            return;

        static auto datamap_size = std::max< int >( desc_map->packed_size, 4 );

        if ( !this->data )
            this->data = new uint8_t[ datamap_size ];

        this->entity = entity;
        this->datamap = entity->get_pred_desc_map( );
        this->pre_action = pre_action;
        this->post_action = post_action;
    }

    __inline void store( ) {
        if ( !this->entity || !this->datamap )
            return;

        pred_copy = c_prediction_copy( PC_EVERYTHING, static_cast< uint8_t * >( this->data ), true, reinterpret_cast< const uint8_t * >( this->entity ), false, transferdata_copyonly );
        pred_copy.transfer_data( this->pre_action, this->entity->index( ), this->datamap );
    }

    __inline void apply( ) {
        if ( !this->entity || !this->datamap )
            return;

        pred_copy = c_prediction_copy( PC_EVERYTHING, reinterpret_cast< uint8_t * >( this->entity ), false, static_cast< const uint8_t * >( this->data ), true, transferdata_copyonly );
        pred_copy.transfer_data( this->post_action, this->entity->index( ), this->datamap );
    }

    c_base_entity *entity;
    static uint8_t *data;
    datamap_t *datamap = nullptr;
    c_prediction_copy pred_copy;
    const char *pre_action;
    const char *post_action;
};

class prediction_context {
public:
    void post_think( c_base_entity *player );
    void start( c_user_cmd *ucmd );
    void finish( c_user_cmd *ucmd );
    void setup_move_data( c_cs_player *player, c_move_data *move_data );
    bool predict_player_entity( c_cs_player *player );
    void restore( );
    void simulate_tick( );

    prediction_data &get_prediction_info( const int current_command );

private:
    uintptr_t prediction_player;
    uintptr_t prediction_seed;
    c_cs_player *target = nullptr;

public:
    int backup_tick_base = 0.f;
    float backup_curtime = 0.f;
    float backup_frametime = 0.f;
    float weapon_cycle;
    float weapon_sequence;
    float weapon_animtime;
    int weapon_model_index = 0;
    int sequence = 0;
    int animation_parity = 0;
    float ideal_inaccuracy = 0.f;

    c_user_cmd dummy_cmd;
    std::array< prediction_data, 90 > pred_data;

    vector_3d velocity = { };
    int predicted_flags = 0;
    bool backup_in_prediction = false;
    bool backup_first_time_predicted = false;
    c_move_data move_data = { };
}; 

inline prediction_context g_prediction_context = { };