#include "prediction_manager.hpp"
#include <features/animations/animation_sync.hpp>
#include <sdk/other/prediction_copy.hpp>

uint8_t *datamap_util::data = nullptr;

class player_backup {
public:
    inline void store( c_cs_player *player ) {
        origin = player->origin( );
        velocity = player->velocity( );
        base_velocity = player->base_velocity( );
        view_offset = player->view_offset( );
        ground_entity = player->ground_entity( );
        flags = player->flags( );
        ducked = player->ducked( );
        ducking = player->ducking( );
        in_duck_jump = player->in_duck_jump( );
        model_scale = player->model_scale( );
        max_speed = player->max_speed( );
    }

    inline void restore( c_cs_player *player ) {
        player->origin( ) = origin;
        player->velocity( ) = velocity;
        player->base_velocity( ) = base_velocity;
        player->view_offset( ) = view_offset;
        player->ground_entity( ) = ground_entity;
        player->flags( ) = flags;
        player->ducked( ) = ducked;
        player->ducking( ) = ducking;
        player->in_duck_jump( ) = in_duck_jump;
        player->model_scale( ) = model_scale;
        player->max_speed( ) = max_speed;
    }

public:
    vector_3d origin;
    vector_3d velocity;
    vector_3d base_velocity;
    vector_3d view_offset;
    int ground_entity;
    int flags;
    float duck_time;
    float duck_jump_time;
    bool ducked;
    bool ducking;
    bool in_duck_jump;
    float model_scale;
    float max_speed;
    datamap_util dt;
};

void prediction_context::post_think( c_base_entity *player ) {
    g_interfaces.model_cache->begin_lock( );

    static auto post_think_vphysics = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 8B D9 56 57 83 BB" ) ).get< bool( __thiscall * )( c_base_entity * ) >( );
    static auto simulate_player_simulated_entities = signature::find( _xs( "client.dll" ), _xs( "56 8B F1 57 8B BE ? ? ? ? 83 EF 01 78 72 90 8B 86" ) ).get< void( __thiscall * )( void * ) >( );

    if ( player->alive( ) ) {
        utils::get_method< void( __thiscall * )( void * ) >( player, 329 )( player );

        if ( player->flags( ) & player_flags::on_ground )
            player->fall_velocity( ) = 0.f;

        if ( player->sequence( ) == -1 )
            player->set_sequence( 0 );

        utils::get_method< void( __thiscall * )( void * ) >( player, 214 )( player );
        post_think_vphysics( player );
    }

    simulate_player_simulated_entities( player );
    g_interfaces.model_cache->end_lock( );
}

void prediction_context::start( c_user_cmd *ucmd ) {
    if ( !globals::local_player || !ucmd || !g_interfaces.move_helper )
        return;

    static bool unlocked_fakelag = false;

    if ( !unlocked_fakelag ) {
        auto cl_move_clamp = signature::find( _xs( "engine.dll" ), _xs( "B8 ? ? ? ? 3B F0 0F 4F F0 89 5D FC" ) ).add( 0x1 ).get< void * >( );
        unsigned long protect = 0;

        VirtualProtect( ( void * ) cl_move_clamp, 4, PAGE_EXECUTE_READWRITE, &protect );
        *( std::uint32_t * ) cl_move_clamp = 62;
        VirtualProtect( ( void * ) cl_move_clamp, 4, protect, &protect );
        unlocked_fakelag = true;
    }

    globals::local_player->current_cmd( ) = ucmd;
    globals::local_player->last_cmd( ) = *ucmd;

    if ( !prediction_player || !prediction_seed ) {
        prediction_seed = signature::find( _xs( "client.dll" ), _xs( "8B 47 40 A3" ) ).add( 4 ).deref( ).get< std::uintptr_t >( );
        prediction_player = signature::find( _xs( "client.dll" ), _xs( "0F 5B C0 89 35" ) ).add( 5 ).deref( ).get< std::uintptr_t >( );
    }

    *reinterpret_cast< int * >( prediction_seed ) = ucmd ? ucmd->random_seed : -1;
    *reinterpret_cast< c_cs_player ** >( prediction_player ) = globals::local_player;

    backup_curtime = g_interfaces.global_vars->curtime;
    backup_frametime = g_interfaces.global_vars->frametime;

    g_interfaces.global_vars->curtime = globals::local_player->tick_base( ) * g_interfaces.global_vars->interval_per_tick;

    backup_first_time_predicted = g_interfaces.prediction->first_time_predicted;
    backup_in_prediction = g_interfaces.prediction->in_prediction;

    g_interfaces.prediction->first_time_predicted = false;
    g_interfaces.prediction->in_prediction = true;

    ucmd->buttons |= globals::local_player->button_forced( );
    //	ucmd->m_buttons &= ~globals::local_player->button_disabled ( );

    g_interfaces.move_helper->set_host( globals::local_player );
    g_interfaces.game_movement->start_track_prediction_errors( globals::local_player );

    velocity = globals::local_player->velocity( );
    predicted_flags = globals::local_player->flags( );

    if ( ucmd->weapon_select != 0 && globals::local_weapon ) {
        auto data = globals::local_weapon_data;

        if ( data )
            globals::local_player->select_item( data->weapon_name, ucmd->weapon_subtype );
    }

    const auto buttons_changed = ucmd->buttons ^ globals::local_player->button_last( );

    globals::local_player->buttons( ) = globals::local_player->button_last( );
    globals::local_player->button_last( ) = ucmd->buttons;
    globals::local_player->button_pressed( ) = ucmd->buttons & buttons_changed;
    globals::local_player->button_released( ) = buttons_changed & ~ucmd->buttons;

    g_interfaces.prediction->check_moving_ground( globals::local_player, g_interfaces.global_vars->frametime );
    g_interfaces.prediction->set_local_view_angles( ucmd->view_angles );

    if ( globals::local_player->physics_run_think( 0 ) )
        globals::local_player->pre_think( );

    const auto think_tick = globals::local_player->think_tick( );

    if ( think_tick > 0 && think_tick <= globals::local_player->tick_base( ) ) {
        globals::local_player->think_tick( ) = -1;
        static auto set_next_think = signature::find( "client.dll", _xs( "55 8B EC 56 57 8B F9 8B B7 ? ? ? ? 8B C6 C1 E8 16 24 01 74 18" ) ).get< void( __thiscall * )( void *, int ) >( );
        set_next_think( globals::local_player, 0 );
        globals::local_player->think( );
    }

    g_interfaces.prediction->setup_move( globals::local_player, ucmd, g_interfaces.move_helper, &move_data );
    g_interfaces.game_movement->process_movement( globals::local_player, &move_data );

    g_interfaces.prediction->finish_move( globals::local_player, ucmd, &move_data );
    g_interfaces.move_helper->process_impacts( );

    post_think( globals::local_player );
    
    if ( globals::local_weapon ) {
        auto weapon_data = globals::local_weapon_data;

        if ( weapon_data ) {
            if ( !globals::local_weapon->is_grenade( ) )
                globals::local_weapon->update_accuracy_penalty( );

            auto weapon_id = globals::local_weapon->item_definition_index( );
            auto is_special_weapon = weapon_id == 9 || weapon_id == 11 || weapon_id == 38 || weapon_id == 40;

            ideal_inaccuracy = 0.0f;

            if ( !( globals::local_player->flags( ) & player_flags::on_ground ) ) {
                if ( weapon_id == weapons::ssg08 && !( globals::local_player->flags( ) & player_flags::on_ground ) )
                    ideal_inaccuracy = 0.00875f;

                else if ( globals::local_player->flags( ) & player_flags::ducking ) {
                    auto weapon_data = globals::local_weapon_data;

                    if ( weapon_data ) {
                        if ( is_special_weapon )
                            ideal_inaccuracy = weapon_data->inaccuracy_crouch_alt;
                        else
                            ideal_inaccuracy = weapon_data->inaccuracy_crouch;
                    }
                }

                else if ( is_special_weapon )
                    ideal_inaccuracy = weapon_data->inaccuracy_stand_alt;
                else
                    ideal_inaccuracy = weapon_data->inaccuracy_stand;
            }
        }

        g_animations.generate_shoot_position( );
    }
}

void prediction_context::finish( c_user_cmd *ucmd ) {
    if ( !globals::local_player || !ucmd || !g_interfaces.move_helper )
        return;

    g_interfaces.game_movement->finish_track_prediction_errors( globals::local_player );
    g_interfaces.move_helper->set_host( nullptr );
    globals::local_player->current_cmd( ) = nullptr;

    *reinterpret_cast< int * >( prediction_seed ) = -1;
    *reinterpret_cast< c_cs_player ** >( prediction_player ) = nullptr;

    g_interfaces.game_movement->reset( );

    g_interfaces.global_vars->curtime = backup_curtime;
    g_interfaces.global_vars->frametime = backup_frametime;

    g_interfaces.prediction->first_time_predicted = backup_first_time_predicted;
    g_interfaces.prediction->in_prediction = backup_in_prediction;
}

static player_backup backup_player_data = { };

void prediction_context::setup_move_data( c_cs_player *player, c_move_data *move_data ) {
    if ( !player || !move_data )
        return;

    move_data->first_run_of_functions = false;
    move_data->game_code_moved_player = false;
    move_data->player_handle = player->get_ref_handle( );
    move_data->velocity = player->velocity( );
    move_data->abs_origin = player->origin( );
    move_data->view_angles = { 0.0f, math::velocity_to_angles( move_data->velocity ).y, 0.0f };
    move_data->max_speed = player->max_speed( );

    move_data->constraint_center = { };
    move_data->constraint_radius = 0.f;
    move_data->constraint_speed_factor = 0.f;

    //if ( backup_player_data.flags & player_flags::ducking )
    //    move_data->max_speed *= 0.3333f;

    move_data->client_max_speed = move_data->max_speed;

    //vector_3d forward, right;
    //math::angle_vectors( move_data->view_angles, &forward, &right, nullptr );

    move_data->forward_move = 450.0f;
    move_data->side_move = 450.0f;
   
    g_interfaces.game_movement->setup_movement_bounds( move_data );
}

bool prediction_context::predict_player_entity( c_cs_player *player ) {
    if ( !g_interfaces.game_movement || !player )
        return false;

    this->target = player;
    this->target->current_command( ) = &dummy_cmd;
    this->target->last_cmd( ) = dummy_cmd;

    backup_player_data.store( player );

    backup_in_prediction = g_interfaces.prediction->in_prediction;
    backup_first_time_predicted = g_interfaces.prediction->first_time_predicted;
    backup_frametime = g_interfaces.global_vars->frametime;

    if ( player->flags( ) & player_flags::ducking ) {
        player->flags( ) &= ~player_flags::ducking;
        player->ducked( ) = true;
        player->ducking( ) = false;
        player->in_duck_jump( ) = false;
    }

    if ( player != globals::local_player )
        player->ground_entity( ) = 0;

    setup_move_data( this->target, &this->move_data );
  
    return true;
}

void prediction_context::restore( ) {
    if ( !g_interfaces.game_movement || !this->target )
        return;

    this->target->current_command( ) = nullptr;

    backup_player_data.restore( this->target );

    g_interfaces.prediction->in_prediction = backup_in_prediction;
    g_interfaces.prediction->first_time_predicted = backup_first_time_predicted;
    g_interfaces.global_vars->frametime = backup_frametime;

    this->target = nullptr;

    memset( &this->move_data, 0, sizeof( c_move_data ) );
    memset( &backup_player_data, 0, sizeof( backup_player_data ) );
}

void prediction_context::simulate_tick( ) {
    if ( !g_interfaces.game_movement || !this->target )
        return;

    g_interfaces.prediction->in_prediction = true;
    g_interfaces.prediction->first_time_predicted = false;
    g_interfaces.global_vars->frametime = g_interfaces.prediction->engine_paused ? 0.0f : game::ticks_to_time( 1 );

    g_interfaces.game_movement->process_movement( this->target, &this->move_data );
}

prediction_data &prediction_context::get_prediction_info( const int current_command ) {
    return pred_data[ current_command % 150 ];
}