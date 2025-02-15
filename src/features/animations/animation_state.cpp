#include "animation_state.hpp"

enum animtag_indices {
    ANIMTAG_INVALID = -1,
    ANIMTAG_UNINITIALIZED = 0,
    ANIMTAG_STARTCYCLE_N,
    ANIMTAG_STARTCYCLE_NE,
    ANIMTAG_STARTCYCLE_E,
    ANIMTAG_STARTCYCLE_SE,
    ANIMTAG_STARTCYCLE_S,
    ANIMTAG_STARTCYCLE_SW,
    ANIMTAG_STARTCYCLE_W,
    ANIMTAG_STARTCYCLE_NW,
    ANIMTAG_AIMLIMIT_YAWMIN_IDLE,
    ANIMTAG_AIMLIMIT_YAWMAX_IDLE,
    ANIMTAG_AIMLIMIT_YAWMIN_WALK,
    ANIMTAG_AIMLIMIT_YAWMAX_WALK,
    ANIMTAG_AIMLIMIT_YAWMIN_RUN,
    ANIMTAG_AIMLIMIT_YAWMAX_RUN,
    ANIMTAG_AIMLIMIT_YAWMIN_CROUCHIDLE,
    ANIMTAG_AIMLIMIT_YAWMAX_CROUCHIDLE,
    ANIMTAG_AIMLIMIT_YAWMIN_CROUCHWALK,
    ANIMTAG_AIMLIMIT_YAWMAX_CROUCHWALK,
    ANIMTAG_AIMLIMIT_PITCHMIN_IDLE,
    ANIMTAG_AIMLIMIT_PITCHMAX_IDLE,
    ANIMTAG_AIMLIMIT_PITCHMIN_WALKRUN,
    ANIMTAG_AIMLIMIT_PITCHMAX_WALKRUN,
    ANIMTAG_AIMLIMIT_PITCHMIN_CROUCH,
    ANIMTAG_AIMLIMIT_PITCHMAX_CROUCH,
    ANIMTAG_AIMLIMIT_PITCHMIN_CROUCHWALK,
    ANIMTAG_AIMLIMIT_PITCHMAX_CROUCHWALK,
    ANIMTAG_WEAPON_POSTLAYER,
    ANIMTAG_FLASHBANG_PASSABLE,
    ANIMTAG_COUNT
};

void c_animation_state_rebuilt::try_initiate_animation( c_cs_player *player, int layer, int activity, c_utl_vector< uint16_t > modifiers ) {
    static const auto find_mapping = signature::find( _xs( "server.dll" ), _xs( "55 8B EC 83 E4 ? 81 EC ? ? ? ? 53 56 57 8B F9 8B 17" ) ).get< void *( __thiscall * ) ( void * ) >( );
    static const auto select_weighted_sequence_from_modifiers = signature::find( _xs( "server.dll" ), _xs( "55 8B EC 83 EC 2C 53 56 8B 75 08 8B D9" ) ).get< int32_t( __thiscall * )( void *, void *, int32_t, const void *, int32_t ) >( );

    const auto mapping = find_mapping( player->cstudio_hdr( ) );
    const auto sequence = select_weighted_sequence_from_modifiers( mapping, player->cstudio_hdr( ), activity, &modifiers[ 0 ], modifiers.Count( ) );

    if ( sequence < 2 )
        return;

    auto &l = player->anim_overlays( )[ layer ];

    l.playback_rate = player->get_sequence_cycle_rate( sequence );
    l.sequence = sequence;
    l.cycle = l.weight = 0.f;
}

c_utl_vector< uint16_t > c_animation_state_rebuilt::build_activity_modifiers( c_cs_player *player ) {
    activity_modifiers_wrapper modifier_wrapper{ };

    const auto state = player->anim_state( );

    modifier_wrapper.add_modifier( state->get_weapon_prefix( ) );

    if ( state->m_flSpeedAsPortionOfRunTopSpeed > .25f )
        modifier_wrapper.add_modifier( "moving" );

    if ( state->m_flAnimDuckAmount > .55f )
        modifier_wrapper.add_modifier( "crouch" );

    return modifier_wrapper.get( );
}

void c_animation_state_rebuilt::set_layer_weight_rate( c_csgo_player_animstate *state, int layer_idx, float previous ) {
    const auto player = state->m_pPlayer;

    if ( !player || !state )
        return;

    if ( state->m_flLastUpdateIncrement == 0 )
        return;

    const auto layer = player->anim_overlays( ) + layer_idx;

    if ( !layer )
        return;

    layer->weight_delta_rate = ( layer->weight - previous ) / state->m_flLastUpdateIncrement;
}

void c_animation_state_rebuilt::set_sequence( c_csgo_player_animstate *state, int layer_idx, int sequence ) {
    const auto player = state->m_pPlayer;

    if ( !player || !state )
        return;

    static auto update_layer_order_preset = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 51 53 56 57 8B F9 83 7F 60 00 0F 84 ? ? ? ? 83" ) ).get< void( __thiscall * )( c_csgo_player_animstate *, int, int ) >( );

    if ( sequence > 1 ) {
        g_interfaces.model_cache->begin_lock( );

        const auto layer = player->anim_overlays( ) + layer_idx;

        if ( !layer )
            return;

        if ( layer->owner && layer->sequence != sequence )
            state->m_pPlayer->invalidate_physics_recursive( 16 );

        layer->sequence = sequence;
        layer->playback_rate = player->get_sequence_cycle_rate( sequence );

        set_cycle( state, layer_idx, 0 );
        set_weight( state, layer_idx, 0 );

        update_layer_order_preset( state, layer_idx, sequence );

        g_interfaces.model_cache->end_lock( );
    }
}

int c_animation_state_rebuilt::select_weighted_sequence( c_csgo_player_animstate *state, int act ) {
    int seq = -1;

    const bool crouching = state->m_flAnimDuckAmount > 0.55f;
    const bool moving = state->m_flSpeedAsPortionOfWalkTopSpeed > 0.25f;

    switch ( act ) {
        case ACT_CSGO_LAND_HEAVY:
            seq = 23;
            if ( crouching )
                seq = 24;
            break;
        case ACT_CSGO_FALL:
            seq = 14;
            break;
        case ACT_CSGO_JUMP:
            seq = moving + 17;
            if ( !crouching )
                seq = moving + 15;
            break;
        case ACT_CSGO_CLIMB_LADDER:
            seq = 13;
            break;
        case ACT_CSGO_LAND_LIGHT:
            seq = 2 * moving + 20;
            if ( crouching ) {
                seq = 21;
                if ( moving )
                    seq = 19;
            }
            break;
        case ACT_CSGO_IDLE_TURN_BALANCEADJUST:
            seq = 4;
            break;
        case ACT_CSGO_IDLE_ADJUST_STOPPEDMOVING:
            seq = 5;
            break;
        case ACT_CSGO_FLASHBANG_REACTION:
            seq = 224;
            if ( crouching )
                seq = 225;
            break;
        case ACT_CSGO_ALIVE_LOOP:
            seq = 8;
            if ( state->m_pWeapon && state->m_pWeapon->get_weapon_data( ) && state->m_pWeapon->get_weapon_data( )->weapon_type == weapon_type::WEAPONTYPE_KNIFE )
                seq = 9;
            break;
        default:
            return -1;
    }

    if ( seq < 2 )
        return -1;

    return seq;
}

void c_animation_state_rebuilt::set_cycle( c_csgo_player_animstate *state, int layer_idx, float cycle ) {
    auto layer = state->m_pPlayer->anim_overlays( ) + layer_idx;

    auto clamp_cycle = []( float in ) {
        in -= int( in );

        if ( in < 0.0f )
            in += 1.0f;
        else if ( in > 1.0f )
            in -= 1.0f;

        return in;
    };

    const auto clamped_cycle = clamp_cycle( cycle );

    if ( layer->owner && layer->cycle != clamped_cycle ) {
        state->m_pPlayer->invalidate_physics_recursive( 8 );
    }

    layer->cycle = clamped_cycle;
}

void c_animation_state_rebuilt::set_weight_delta_rate( c_csgo_player_animstate *state, int layer_idx, float old_weight ) {
    if ( state->m_flLastUpdateIncrement != 0.0f ) {
        const auto layer = state->m_pPlayer->anim_overlays( ) + layer_idx;

        float weight_delta_rate = ( layer->weight - old_weight ) / state->m_flLastUpdateIncrement;

        if ( layer->weight_delta_rate != weight_delta_rate )
            layer->weight_delta_rate = weight_delta_rate;
    }
}

void c_animation_state_rebuilt::increment_layer_cycle_weight_rate_generic( c_csgo_player_animstate *state, int layer_idx ) {
    float prev_weight = state->m_pPlayer->anim_overlays( )[ layer_idx ].weight;
    increment_layer_cycle( state, layer_idx, false );
    set_weight( state, layer_idx, get_layer_ideal_weight_from_sequence_cycle( state, layer_idx ) );
    set_weight_delta_rate( state, layer_idx, prev_weight );
}

void c_animation_state_rebuilt::handle_animation_events( c_cs_player *player, c_csgo_player_animstate *animstate ) {
    if ( !player || !animstate )
        return;
    const bool on_ground = ( globals::local_player->flags( ) & player_flags::on_ground ) != 0;
    const bool landed_on_ground_this_frame = on_ground && !animstate->m_bOnGround;

    /* rebuild some missing anim code */
    const bool stopped_moving_this_frame = animstate->m_flVelocityLengthXY <= 0.0f && animstate->m_flDurationStill <= 0.0f;
    const bool previously_on_ladder = animstate->m_bOnLadder;
    const bool on_ladder = animstate->m_pPlayer->move_type( ) == move_types::ladder;
    const bool started_laddering_this_frame = ( !previously_on_ladder && on_ladder );
    const bool stopped_laddering_this_frame = ( previously_on_ladder && !on_ladder );

    /* only for localplayer (we dont have enemy usercmds) */
    if ( globals::local_player && animstate->m_pPlayer == globals::local_player ) {
        /* FROM src/game/prediction.cpp */
        const uint32_t buttons = *reinterpret_cast< uint32_t * >( reinterpret_cast< uintptr_t >( animstate->m_pPlayer ) + 0x3208 );

        bool move_right = ( buttons & ( buttons::move_right ) ) != 0;
        bool move_left = ( buttons & ( buttons::move_left ) ) != 0;
        bool move_forward = ( buttons & ( buttons::forward ) ) != 0;
        bool move_backward = ( buttons & ( buttons::back ) ) != 0;

        vector_3d forward, right;
        valve_math::angle_vectors( vector_3d( 0, animstate->m_flFootYaw, 0 ), &forward, &right, nullptr );

        auto right_normalized = math::normalize_angle( right );

        const float vel_to_right_dot = glm::dot( animstate->m_vecVelocityNormalizedNonZero, right_normalized );
        const float vel_to_forward_dot = glm::dot( animstate->m_vecVelocityNormalizedNonZero, forward );

        const bool strafe_right = ( animstate->m_flSpeedAsPortionOfWalkTopSpeed >= 0.73f && move_right && !move_left && vel_to_right_dot < -0.63f );
        const bool strafe_left = ( animstate->m_flSpeedAsPortionOfWalkTopSpeed >= 0.73f && move_left && !move_right && vel_to_right_dot > 0.63f );
        const bool strafe_forward = ( animstate->m_flSpeedAsPortionOfWalkTopSpeed >= 0.65f && move_forward && !move_backward && vel_to_forward_dot < -0.55f );
        const bool strafe_back = ( animstate->m_flSpeedAsPortionOfWalkTopSpeed >= 0.65f && move_backward && !move_forward && vel_to_forward_dot > 0.55f );

        *reinterpret_cast< bool * >( reinterpret_cast< uintptr_t >( animstate->m_pPlayer ) + 0x39E0 ) = ( strafe_right || strafe_left || strafe_forward || strafe_back );

        if ( ( animstate->m_flLadderWeight > 0.0f || on_ladder ) && started_laddering_this_frame )
            set_sequence( animstate, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, select_weighted_sequence( animstate, 987 ) );

        if ( on_ground ) {
            bool next_landing = false;
            if ( !animstate->m_bLanding && ( landed_on_ground_this_frame || stopped_laddering_this_frame ) ) {
                set_sequence( animstate, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, select_weighted_sequence( animstate, ( animstate->m_flDurationInAir > 1.0f ) ? 989 : 988 ) );
                next_landing = true;
            }

            if ( next_landing && get_layer_activity( animstate, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ) != 987 ) {
                m_bJumping = false;

                /* some client code here */
                auto &layer = animstate->m_pPlayer->anim_overlays( )[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ];

                /* dont actually mess up the value */
                const float backup_cycle = layer.cycle;

                /* run this calculation ahead of time so we dont get 1-tick-late landing */
                increment_layer_cycle( animstate, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, false );

                if ( is_layer_sequence_completed( animstate, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ) )
                    next_landing = false;

                layer.cycle = backup_cycle;
            }

            if ( !next_landing && !m_bJumping && animstate->m_flLadderWeight <= 0.0f ) {
                auto &layer = animstate->m_pPlayer->anim_overlays( )[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ];
                layer.weight = 0.0f;
            }
        } else if ( !on_ladder ) {


            if ( !animstate->m_bOnGround || stopped_laddering_this_frame ) {
                if ( m_bJumping ) {
                    //play_animation ( animstate, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, 986 );
                    set_sequence( animstate, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, select_weighted_sequence( animstate, ACT_CSGO_FALL ) );
                }
            }
            //#ifndef CLIENT_DLL
            //		Msg( "%f\n", m_flDurationInAir );
            //#endif

            increment_layer_cycle( animstate, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, false );
        }
    }
}

void c_animation_state_rebuilt::update_animation_state( c_csgo_player_animstate *animstate, const vector_3d &angles, int tick, bool handle_events ) {
    const c_csgo_player_animstate backup_animstate = *animstate;

    const bool on_ground = ( globals::local_player->flags( ) & player_flags::on_ground ) != 0;
    const bool landed_on_ground_this_frame = on_ground && !animstate->m_bOnGround;

    bool &m_bJumping = animstate->m_bFlashed;

    if ( animstate->m_pPlayer == globals::local_player && !on_ground && animstate->m_bOnGround && animstate->m_pPlayer->velocity( ).z > 0.0f ) {
        m_bJumping = true;
        set_sequence( animstate, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, select_weighted_sequence( animstate, ACT_CSGO_JUMP ) );
    }

    /* rebuild some missing anim code */
    const bool stopped_moving_this_frame = animstate->m_flVelocityLengthXY <= 0.0f && animstate->m_flDurationStill <= 0.0f;
    const bool previously_on_ladder = animstate->m_bOnLadder;
    const bool on_ladder = animstate->m_pPlayer->move_type( ) == move_types::ladder;
    const bool started_laddering_this_frame = ( !previously_on_ladder && on_ladder );
    const bool stopped_laddering_this_frame = ( previously_on_ladder && !on_ladder );

    /* CCSGOPlayerAnimState::SetupVelocity() ANIMATION_LAYER_ADJUST calculations */
    if ( !animstate->m_bAdjustStarted && stopped_moving_this_frame && on_ground && !on_ladder && !animstate->m_bLanding && animstate->m_flStutterStep < 50.0f ) {
        set_sequence( animstate, ANIMATION_LAYER_ADJUST, select_weighted_sequence( animstate, 980 ) );
        animstate->m_bAdjustStarted = true;
    }

    const int adjust_activity = get_layer_activity( animstate, ANIMATION_LAYER_ADJUST );

    if ( adjust_activity == 980 || adjust_activity == 979 ) {
        if ( animstate->m_bAdjustStarted && animstate->m_flSpeedAsPortionOfCrouchTopSpeed <= 0.25f ) {
            increment_layer_cycle_weight_rate_generic( animstate, ANIMATION_LAYER_ADJUST );
            animstate->m_bAdjustStarted = !is_layer_sequence_completed( animstate, ANIMATION_LAYER_ADJUST );
        } else {
            animstate->m_bAdjustStarted = false;

            auto &layer = animstate->m_pPlayer->anim_overlays( )[ ANIMATION_LAYER_ADJUST ];
            const float weight = layer.weight;
            layer.weight = valve_math::approach( 0.0f, weight, animstate->m_flLastUpdateIncrement * 5.0f );
            set_weight_delta_rate( animstate, ANIMATION_LAYER_ADJUST, weight );
        }
    }

    auto weapon = g_interfaces.entity_list->get_client_entity_from_handle< c_cs_weapon_base * >( animstate->m_pPlayer->weapon_handle( ) );

    if ( !weapon )
        return;

    const auto max_speed_run = weapon ? std::max( weapon->get_weapon_data( )->max_speed, 0.001f ) : CS_PLAYER_SPEED_RUN;
    const auto velocity = animstate->m_pPlayer->velocity( );

    animstate->m_flVelocityLengthXY = std::min< float >( glm::length( velocity ), CS_PLAYER_SPEED_RUN );

    if ( animstate->m_flVelocityLengthXY > 0.0f )
        animstate->m_vecVelocityNormalizedNonZero = math::normalize_angle( velocity );

    animstate->m_flSpeedAsPortionOfWalkTopSpeed = animstate->m_flVelocityLengthXY / ( max_speed_run * CS_PLAYER_SPEED_WALK_MODIFIER );


    std::array< c_animation_layer, 13 > backup_layers;

    /* update animation */
    animstate->update( angles );

    auto &adjust_layer = animstate->m_pPlayer->anim_overlays( )[ ANIMATION_LAYER_ADJUST ];

    if ( adjust_layer.weight > 0.0f ) {
        adjust_layer.cycle = std::clamp( adjust_layer.cycle - animstate->m_flLastUpdateIncrement * adjust_layer.playback_rate, 0.0f, 0.999f );
        adjust_layer.weight = std::clamp( adjust_layer.weight - animstate->m_flLastUpdateIncrement * adjust_layer.weight_delta_rate, 0.0f, 1.0f );
    }

    if ( animstate->m_flVelocityLengthXY <= CS_PLAYER_SPEED_STOPPED && animstate->m_bOnGround && !animstate->m_bOnLadder && !animstate->m_bLanding && animstate->m_flLastUpdateIncrement > 0.0f && abs( valve_math::angle_diff( animstate->m_flFootYawLast, animstate->m_flFootYaw ) / animstate->m_flLastUpdateIncrement > CSGO_ANIM_READJUST_THRESHOLD ) ) {
        set_sequence( animstate, ANIMATION_LAYER_ADJUST, select_weighted_sequence( animstate, ACT_CSGO_IDLE_TURN_BALANCEADJUST ) );
        animstate->m_bAdjustStarted = true;
    }
}

void c_animation_state_rebuilt::set_weight( c_csgo_player_animstate *state, int layer_idx, float weight ) {
    auto layer = state->m_pPlayer->anim_overlays( ) + layer_idx;

    if ( layer->owner && layer->weight != weight ) {
        if ( weight == 0.0f || weight == 0.0f ) {
            state->m_pPlayer->invalidate_physics_recursive( 10 );
        }
    }

    layer->weight = weight;
}

float c_animation_state_rebuilt::get_layer_ideal_weight_from_sequence_cycle( c_csgo_player_animstate *state, int layer_idx ) {
    static auto get_layer_ideal_weight_from_sequence_cycle = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 EC 08 53 56 8B 35 ? ? ? ? 57 8B F9 8B CE 8B 06 FF 90" ) ).get< float( __thiscall * )( c_csgo_player_animstate *, int ) >( );

    return get_layer_ideal_weight_from_sequence_cycle( state, layer_idx );
}

bool c_animation_state_rebuilt::is_layer_sequence_completed( c_csgo_player_animstate *state, int layer_idx ) {
    const auto layer = state->m_pPlayer->anim_overlays( ) + layer_idx;

    if ( layer )
        return ( ( layer->cycle + ( state->m_flLastUpdateIncrement * layer->playback_rate ) ) >= 1 );

    return false;
}

void c_animation_state_rebuilt::increment_layer_cycle( c_csgo_player_animstate *state, int layer_idx, bool allow_loop ) {
    const auto layer = state->m_pPlayer->anim_overlays( ) + layer_idx;

    if ( !layer )
        return;

    if ( abs( layer->playback_rate ) <= 0 )
        return;

    float flCurrentCycle = state->m_pPlayer->anim_overlays( )[ layer_idx ].cycle;
    flCurrentCycle += state->m_flLastUpdateIncrement * state->m_pPlayer->anim_overlays( )[ layer_idx ].playback_rate;

    if ( !allow_loop && flCurrentCycle >= 1 ) {
        flCurrentCycle = 0.999f;
    }

    set_cycle( state, layer_idx, valve_math::clamp_cycle( flCurrentCycle ) );
}

int c_animation_state_rebuilt::get_layer_activity( c_csgo_player_animstate *state, AnimationLayer_t layer_idx ) {
    static auto get_layer_activity = signature::find( _xs( "client.dll" ), _xs( "51 53 56 8B 35 ? ? ? ? 57 8B F9 8B CE 8B 06 FF 90 ? ? ? ? 8B 7F 60 83 BF" ) ).get< int( __thiscall * )( c_csgo_player_animstate *, int ) >( );

    return get_layer_activity( state, layer_idx );
}