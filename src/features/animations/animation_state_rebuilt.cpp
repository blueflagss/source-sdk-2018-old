#include "animation_state_rebuilt.hpp"

#define CSGO_ANIM_AIMMATRIX_DEFAULT_YAW_MAX 58.0f
#define CSGO_ANIM_AIMMATRIX_DEFAULT_YAW_MIN -58.0f
#define CSGO_ANIM_AIMMATRIX_DEFAULT_PITCH_MAX 90.0f
#define CSGO_ANIM_AIMMATRIX_DEFAULT_PITCH_MIN -90.0f
#define CSGO_ANIM_LOWER_CATCHUP_IDLE 100.0f
#define CSGO_ANIM_AIM_NARROW_WALK 0.8f
#define CSGO_ANIM_AIM_NARROW_RUN 0.5f
#define CSGO_ANIM_AIM_NARROW_CROUCHMOVING 0.5f
#define CSGO_ANIM_LOWER_CATCHUP_WITHIN 3.0f
#define CSGO_ANIM_LOWER_REALIGN_DELAY 1.1f
#define CSGO_ANIM_READJUST_THRESHOLD 120.0f
#define EIGHT_WAY_WIDTH 22.5f

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

void c_animation_state_rebuilt::set_sequence( c_csgo_player_animstate *state, int layer_idx, int sequence ) {
    const auto player = state->m_pPlayer;

    if ( !player || !state )
        return;

    static auto CCSGOPlayerAnimState_UpdateLayerOrderPreset = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 51 53 56 57 8B F9 83 7F 60 00 0F 84 ? ? ? ? 83" ) ).get< void( __thiscall * )( c_csgo_player_animstate *, int, int ) >( );

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

        CCSGOPlayerAnimState_UpdateLayerOrderPreset( state, layer_idx, sequence );

        g_interfaces.model_cache->end_lock( );
    }
}

void c_animation_state_rebuilt::setup_movement( c_csgo_player_animstate *state ) {
    auto player = state->m_pPlayer;

    if ( !state || !player )
        return;

    g_interfaces.model_cache->begin_lock( );

    bool &m_bJumping = state->m_bFlashed;

    if ( !( player->flags( ) & player_flags::on_ground ) && state->m_bOnGround && player == globals::local_player && state->m_vecVelocity.z > 0.0f ) {
        m_bJumping = true;
        set_sequence( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, select_weighted_sequence( state, ACT_CSGO_JUMP ) );
    }

    if ( state->m_flWalkToRunTransition > 0 && state->m_flWalkToRunTransition < 1 ) {
        if ( state->m_bWalkToRunTransitionState == ANIM_TRANSITION_WALK_TO_RUN ) {
            state->m_flWalkToRunTransition += state->m_flLastUpdateIncrement * CSGO_ANIM_WALK_TO_RUN_TRANSITION_SPEED;
        } else {
            state->m_flWalkToRunTransition -= state->m_flLastUpdateIncrement * CSGO_ANIM_WALK_TO_RUN_TRANSITION_SPEED;
        }
        state->m_flWalkToRunTransition = std::clamp< float >( state->m_flWalkToRunTransition, 0, 1 );
    }

    if ( state->m_flVelocityLengthXY > ( CS_PLAYER_SPEED_RUN * CS_PLAYER_SPEED_WALK_MODIFIER ) && state->m_bWalkToRunTransitionState == ANIM_TRANSITION_RUN_TO_WALK ) {
        state->m_bWalkToRunTransitionState = ANIM_TRANSITION_WALK_TO_RUN;
        state->m_flWalkToRunTransition = std::max< float >( 0.01f, state->m_flWalkToRunTransition );
    } else if ( state->m_flVelocityLengthXY < ( CS_PLAYER_SPEED_RUN * CS_PLAYER_SPEED_WALK_MODIFIER ) && state->m_bWalkToRunTransitionState == ANIM_TRANSITION_WALK_TO_RUN ) {
        state->m_bWalkToRunTransitionState = ANIM_TRANSITION_RUN_TO_WALK;
        state->m_flWalkToRunTransition = std::min< float >( 0.99f, state->m_flWalkToRunTransition );
    }

    if ( state->m_nAnimstateModelVersion < 2 ) {
        state->m_tPoseParamMappings[ PLAYER_POSE_PARAM_RUN ].SetValue( state->m_pPlayer, state->m_flWalkToRunTransition );
    } else {
        state->m_tPoseParamMappings[ PLAYER_POSE_PARAM_MOVE_BLEND_WALK ].SetValue( state->m_pPlayer, ( 1.0f - state->m_flWalkToRunTransition ) * ( 1.0f - state->m_flAnimDuckAmount ) );
        state->m_tPoseParamMappings[ PLAYER_POSE_PARAM_MOVE_BLEND_RUN ].SetValue( state->m_pPlayer, ( state->m_flWalkToRunTransition ) * ( 1.0f - state->m_flAnimDuckAmount ) );
        state->m_tPoseParamMappings[ PLAYER_POSE_PARAM_MOVE_BLEND_CROUCH_WALK ].SetValue( state->m_pPlayer, state->m_flAnimDuckAmount );
    }

    char szWeaponMoveSeq[ MAX_ANIMSTATE_ANIMNAME_CHARS ];
    sprintf_s( szWeaponMoveSeq, "move_%s", state->get_weapon_prefix( ) );

    int nWeaponMoveSeq = state->m_pPlayer->lookup_sequence( szWeaponMoveSeq );

    if ( nWeaponMoveSeq == -1 ) {
        nWeaponMoveSeq = state->m_pPlayer->lookup_sequence( "move" );
    }

    if ( state->m_pPlayer->move_state( ) != state->m_nPreviousMoveState ) {
        state->m_flStutterStep += 10;
    }

    state->m_nPreviousMoveState = state->m_pPlayer->move_state( );
    state->m_flStutterStep = std::clamp< float >( valve_math::approach( 0, state->m_flStutterStep, state->m_flLastUpdateIncrement * 40 ), 0, 100 );

    float flTargetMoveWeight = valve_math::Lerp( state->m_flAnimDuckAmount, std::clamp< float >( state->m_flSpeedAsPortionOfWalkTopSpeed, 0, 1 ), std::clamp< float >( state->m_flSpeedAsPortionOfCrouchTopSpeed, 0, 1 ) );
    if ( state->m_flMoveWeight <= flTargetMoveWeight ) {
        state->m_flMoveWeight = flTargetMoveWeight;
    } else {
        state->m_flMoveWeight = valve_math::approach( flTargetMoveWeight, state->m_flMoveWeight, state->m_flLastUpdateIncrement * valve_math::remap_val_clamped( state->m_flStutterStep, 0.0f, 100.0f, 2, 20 ) );
    }

    vector_3d vecMoveYawDir;
    valve_math::angle_vectors( vector_3d( 0, valve_math::angle_normalize( state->m_flFootYaw + state->m_flMoveYaw + 180.0f ), 0 ), &vecMoveYawDir, nullptr, nullptr );
    float flYawDeltaAbsDot = abs( glm::dot( state->m_vecVelocityNormalizedNonZero, vecMoveYawDir ) );
    state->m_flMoveWeight *= valve_math::bias( flYawDeltaAbsDot, 0.2 );

    float flMoveWeightWithAirSmooth = state->m_flMoveWeight * state->m_flInAirSmoothValue;

    flMoveWeightWithAirSmooth *= std::max< float >( ( 1.0f - state->m_pPlayer->anim_overlays( )[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ].weight ), 0.55f );

    float flMoveCycleRate = 0;
    if ( state->m_flVelocityLengthXY > 0 ) {
        flMoveCycleRate = state->m_pPlayer->get_sequence_cycle_rate( nWeaponMoveSeq );
        float flSequenceGroundSpeed = std::max< float >( state->m_pPlayer->get_sequence_move_dist( state->m_pPlayer->get_model_ptr( ), nWeaponMoveSeq ) / ( 1.0f / flMoveCycleRate ), 0.001f );
        flMoveCycleRate *= state->m_flVelocityLengthXY / flSequenceGroundSpeed;

        flMoveCycleRate *= valve_math::Lerp( state->m_flWalkToRunTransition, 1.0f, CSGO_ANIM_RUN_ANIM_PLAYBACK_MULTIPLIER );
    }

    float flLocalCycleIncrement = ( flMoveCycleRate * state->m_flLastUpdateIncrement );
    state->m_flPrimaryCycle = valve_math::clamp_cycle( state->m_flPrimaryCycle + flLocalCycleIncrement );

    flMoveWeightWithAirSmooth = std::clamp< float >( flMoveWeightWithAirSmooth, 0, 1 );
    update_anim_layer( state, ANIMATION_LAYER_MOVEMENT_MOVE, nWeaponMoveSeq, flLocalCycleIncrement, flMoveWeightWithAirSmooth, state->m_flPrimaryCycle );

    bool moveRight = ( globals::user_cmd->buttons & ( buttons::move_right ) ) != 0;
    bool moveLeft = ( globals::user_cmd->buttons & ( buttons::move_left ) ) != 0;
    bool moveForward = ( globals::user_cmd->buttons & ( buttons::forward ) ) != 0;
    bool moveBackward = ( globals::user_cmd->buttons & ( buttons::back ) ) != 0;

    vector_3d vecForward;
    vector_3d vecRight;
    valve_math::angle_vectors( vector_3d( 0, state->m_flFootYaw, 0 ), &vecForward, &vecRight, NULL );
    math::normalize_place( vecRight );
    float flVelToRightDot = glm::dot( state->m_vecVelocityNormalizedNonZero, vecRight );
    float flVelToForwardDot = glm::dot( state->m_vecVelocityNormalizedNonZero, vecForward );

    bool bStrafeRight = ( state->m_flSpeedAsPortionOfWalkTopSpeed >= 0.73f && moveRight && !moveLeft && flVelToRightDot < -0.63f );
    bool bStrafeLeft = ( state->m_flSpeedAsPortionOfWalkTopSpeed >= 0.73f && moveLeft && !moveRight && flVelToRightDot > 0.63f );
    bool bStrafeForward = ( state->m_flSpeedAsPortionOfWalkTopSpeed >= 0.65f && moveForward && !moveBackward && flVelToForwardDot < -0.55f );
    bool bStrafeBackward = ( state->m_flSpeedAsPortionOfWalkTopSpeed >= 0.65f && moveBackward && !moveForward && flVelToForwardDot > 0.55f );

    state->m_pPlayer->strafing( ) = ( bStrafeRight || bStrafeLeft || bStrafeForward || bStrafeBackward );

    if ( state->m_pPlayer->strafing( ) ) {
        if ( !state->m_bStrafeChanging ) {
            state->m_flDurationStrafing = 0;
        }

        state->m_bStrafeChanging = true;

        state->m_flStrafeChangeWeight = valve_math::approach( 1, state->m_flStrafeChangeWeight, state->m_flLastUpdateIncrement * 20 );
        state->m_flStrafeChangeCycle = valve_math::approach( 0, state->m_flStrafeChangeCycle, state->m_flLastUpdateIncrement * 10 );

        state->m_tPoseParamMappings[ PLAYER_POSE_PARAM_STRAFE_DIR ].SetValue( state->m_pPlayer, valve_math::angle_normalize( state->m_flMoveYaw ) );
    } else if ( state->m_flStrafeChangeWeight > 0 ) {
        state->m_flDurationStrafing += state->m_flLastUpdateIncrement;

        if ( state->m_flDurationStrafing > 0.08f )
            state->m_flStrafeChangeWeight = valve_math::approach( 0, state->m_flStrafeChangeWeight, state->m_flLastUpdateIncrement * 5 );

        state->m_nStrafeSequence = state->m_pPlayer->lookup_sequence( "strafe" );
        float flRate = state->m_pPlayer->get_sequence_cycle_rate( state->m_nStrafeSequence );
        state->m_flStrafeChangeCycle = std::clamp< float >( state->m_flStrafeChangeCycle + state->m_flLastUpdateIncrement * flRate, 0, 1 );
    }

    if ( state->m_flStrafeChangeWeight <= 0 ) {
        state->m_bStrafeChanging = false;
    }

    bool bPreviousGroundState = state->m_bOnGround;
    state->m_bOnGround = ( state->m_pPlayer->flags( ) & FL_ONGROUND );

    state->m_bLandedOnGroundThisFrame = ( !state->m_bFirstRunSinceInit && bPreviousGroundState != state->m_bOnGround && state->m_bOnGround );
    state->m_bLeftTheGroundThisFrame = ( bPreviousGroundState != state->m_bOnGround && !state->m_bOnGround );

    float flDistanceFell = 0;
    if ( state->m_bLeftTheGroundThisFrame ) {
        state->m_flLeftGroundHeight = state->m_vecPositionCurrent.z;
    }

    if ( state->m_bLandedOnGroundThisFrame ) {
        flDistanceFell = abs( state->m_flLeftGroundHeight - state->m_vecPositionCurrent.z );
        float flDistanceFallNormalizedBiasRange = valve_math::bias( valve_math::remap_val_clamped( flDistanceFell, 12.0f, 72.0f, 0.0f, 1.0f ), 0.4f );

        state->m_flLandAnimMultiplier = std::clamp< float >( valve_math::bias( state->m_flDurationInAir, 0.3f ), 0.1f, 1.0f );
        state->m_flDuckAdditional = std::max< float >( state->m_flLandAnimMultiplier, flDistanceFallNormalizedBiasRange );
    } else {
        state->m_flDuckAdditional = valve_math::approach( 0, state->m_flDuckAdditional, state->m_flLastUpdateIncrement * 2 );
    }

    state->m_flInAirSmoothValue = valve_math::approach( state->m_bOnGround ? 1 : 0, state->m_flInAirSmoothValue, valve_math::Lerp( state->m_flAnimDuckAmount, CSGO_ANIM_ONGROUND_FUZZY_APPROACH, CSGO_ANIM_ONGROUND_FUZZY_APPROACH_CROUCH ) * state->m_flLastUpdateIncrement );
    state->m_flInAirSmoothValue = std::clamp< float >( state->m_flInAirSmoothValue, 0, 1 );

    state->m_flStrafeChangeWeight *= ( 1.0f - state->m_flAnimDuckAmount );
    state->m_flStrafeChangeWeight *= state->m_flInAirSmoothValue;
    state->m_flStrafeChangeWeight = std::clamp< float >( state->m_flStrafeChangeWeight, 0, 1 );

    if ( state->m_nStrafeSequence != -1 )
        update_anim_layer( state, ANIMATION_LAYER_MOVEMENT_STRAFECHANGE, state->m_nStrafeSequence, 0, state->m_flStrafeChangeWeight, state->m_flStrafeChangeCycle );

    bool bPreviouslyOnLadder = state->m_bOnLadder;
    state->m_bOnLadder = !state->m_bOnGround && state->m_pPlayer->move_type( ) == move_types::ladder;
    bool bStartedLadderingThisFrame = ( !bPreviouslyOnLadder && state->m_bOnLadder );
    bool bStoppedLadderingThisFrame = ( bPreviouslyOnLadder && !state->m_bOnLadder );

    if ( state->m_flLadderWeight > 0 || state->m_bOnLadder ) {
        if ( bStartedLadderingThisFrame ) {
            set_sequence( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, select_weighted_sequence( state, ACT_CSGO_CLIMB_LADDER ) );
        }

        if ( abs( state->m_flVelocityLengthZ ) > 100 ) {
            state->m_flLadderSpeed = valve_math::approach( 1, state->m_flLadderSpeed, state->m_flLastUpdateIncrement * 10.0f );
        } else {
            state->m_flLadderSpeed = valve_math::approach( 0, state->m_flLadderSpeed, state->m_flLastUpdateIncrement * 10.0f );
        }
        state->m_flLadderSpeed = std::clamp< float >( state->m_flLadderSpeed, 0, 1 );

        if ( state->m_bOnLadder ) {
            state->m_flLadderWeight = valve_math::approach( 1, state->m_flLadderWeight, state->m_flLastUpdateIncrement * 5.0f );
        } else {
            state->m_flLadderWeight = valve_math::approach( 0, state->m_flLadderWeight, state->m_flLastUpdateIncrement * 10.0f );
        }

        state->m_flLadderWeight = std::clamp< float >( state->m_flLadderWeight, 0, 1 );

        vector_3d vecLadderNormal = state->m_pPlayer->ladder_normal( );
        vector_3d angLadder = math::vector_angle( vecLadderNormal );
        float flLadderYaw = valve_math::angle_diff( angLadder.y, state->m_flFootYaw );
        state->m_tPoseParamMappings[ PLAYER_POSE_PARAM_LADDER_YAW ].SetValue( state->m_pPlayer, flLadderYaw );

        float flLadderClimbCycle = state->m_pPlayer->anim_overlays( )[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ].cycle;
        flLadderClimbCycle += ( state->m_vecPositionCurrent.z - state->m_vecPositionLast.z ) * valve_math::Lerp( state->m_flLadderSpeed, 0.010f, 0.004f );

        state->m_tPoseParamMappings[ PLAYER_POSE_PARAM_LADDER_SPEED ].SetValue( state->m_pPlayer, state->m_flLadderSpeed );

        if ( get_layer_activity( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ) == ACT_CSGO_CLIMB_LADDER ) {
            set_weight( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, state->m_flLadderWeight );
        }

        set_cycle( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, flLadderClimbCycle );

        if ( state->m_bOnLadder ) {
            float flIdealJumpWeight = 1.0f - state->m_flLadderWeight;
            if ( state->m_pPlayer->anim_overlays( )[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ].weight > flIdealJumpWeight ) {
                set_weight( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, flIdealJumpWeight );
            }
        }
    } else {
        state->m_flLadderSpeed = 0;
    }

    if ( state->m_bOnGround ) {
        if ( !state->m_bLanding && ( state->m_bLandedOnGroundThisFrame || bStoppedLadderingThisFrame ) ) {
            set_sequence( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, select_weighted_sequence( state, ( state->m_flDurationInAir > 1 ) ? ACT_CSGO_LAND_HEAVY : ACT_CSGO_LAND_LIGHT ) );
            set_cycle( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, 0 );
            state->m_bLanding = true;
        }
        state->m_flDurationInAir = 0;

        if ( state->m_bLanding && get_layer_activity( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ) != ACT_CSGO_CLIMB_LADDER ) {
            m_bJumping = false;

            increment_layer_cycle( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, false );
            increment_layer_cycle( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, false );

            state->m_tPoseParamMappings[ PLAYER_POSE_PARAM_JUMP_FALL ].SetValue( state->m_pPlayer, 0 );

            if ( is_layer_sequence_completed( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ) ) {
                state->m_bLanding = false;
                set_weight( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, 0 );
                set_weight( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, 0 );
                state->m_flLandAnimMultiplier = 1.0f;
            } else {
                float flLandWeight = get_layer_ideal_weight_from_sequence_cycle( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ) * state->m_flLandAnimMultiplier;

                flLandWeight *= std::clamp< float >( ( 1.0f - state->m_flAnimDuckAmount ), 0.2f, 1.0f );

                set_weight( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, flLandWeight );

                float flCurrentJumpFallWeight = player->anim_overlays( )[ ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ].weight;
                if ( flCurrentJumpFallWeight > 0 ) {
                    state->m_flDurationInAir += state->m_flLastUpdateIncrement;
                    flCurrentJumpFallWeight = valve_math::approach( 0, flCurrentJumpFallWeight, state->m_flLastUpdateIncrement * 10.0f );
                    set_weight( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, flCurrentJumpFallWeight );
                }
            }
        }

        if ( !state->m_bLanding && !m_bJumping && state->m_flLadderWeight <= 0 ) {
            set_weight( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, 0 );
        }
    } else if ( !state->m_bOnLadder ) {
        state->m_bLanding = false;

        if ( state->m_bLeftTheGroundThisFrame || bStoppedLadderingThisFrame ) {
            if ( !m_bJumping ) {
                set_sequence( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, select_weighted_sequence( state, ACT_CSGO_FALL ) );
            }

            state->m_flDurationInAir = 0;
        }

        state->m_flDurationInAir += state->m_flLastUpdateIncrement;

        increment_layer_cycle( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, false );

        float flJumpWeight = state->m_pPlayer->anim_overlays( )[ ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ].weight;
        float flNextJumpWeight = get_layer_ideal_weight_from_sequence_cycle( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL );
        if ( flNextJumpWeight > flJumpWeight ) {
            set_weight( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, flNextJumpWeight );
        }

        float flLingeringLandWeight = state->m_pPlayer->anim_overlays( )[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ].weight;
        if ( flLingeringLandWeight > 0 ) {
            flLingeringLandWeight *= valve_math::smoothstep_bounds( 0.2f, 0.0f, state->m_flDurationInAir );
            set_weight( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, flLingeringLandWeight );
        }

        state->m_tPoseParamMappings[ PLAYER_POSE_PARAM_JUMP_FALL ].SetValue( state->m_pPlayer, std::clamp< float >( valve_math::smoothstep_bounds( 0.72f, 1.52f, state->m_flDurationInAir ), 0, 1 ) );
    }

    g_interfaces.model_cache->end_lock( );
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

void c_animation_state_rebuilt::update_animation_state( c_csgo_player_animstate *state, vector_3d &angles, bool force_update ) {
    static auto *enable_invalidate_bone_cache = signature::find( _xs( "client.dll" ), _xs( "C6 05 ? ? ? ? ? F3 0F 5F 05 ? ? ? ? F3 0F 11 47 ? F3 0F" ) ).add( 0x2 ).deref( ).get< bool * >( );

    if ( !state )
        return;

    const auto player = state->m_pPlayer;

    if ( !player || !player->alive( ) )
        return;

    const auto backup_cur_time = g_interfaces.global_vars->curtime;
    const auto backup_frametime = g_interfaces.global_vars->frametime;
    const auto backup_framecount = g_interfaces.global_vars->framecount;

    angles.x = valve_math::angle_normalize( angles.x + *reinterpret_cast< float * >( reinterpret_cast< uintptr_t >( player ) + 0xB844 ) );

    if ( !force_update && ( state->m_flLastUpdateTime == g_interfaces.global_vars->curtime || state->m_nLastUpdateFrame == g_interfaces.global_vars->framecount ) )
        return;

    state->m_flLastUpdateIncrement = std::max< float >( 0.0f, g_interfaces.global_vars->curtime - state->m_flLastUpdateTime );

    *enable_invalidate_bone_cache = false;

    state->m_flEyeYaw = valve_math::angle_normalize( angles.y );
    state->m_flEyePitch = valve_math::angle_normalize( angles.x );
    state->m_vecPositionCurrent = player->get_abs_origin( );
    state->m_pWeapon = g_interfaces.entity_list->get_client_entity_from_handle< c_cs_weapon_base * >( state->m_pPlayer->weapon_handle( ) );

    if ( state->m_pWeapon != state->m_pWeaponLast || state->m_bFirstRunSinceInit ) {
        *reinterpret_cast< int * >( reinterpret_cast< uintptr_t >( player ) + 0xA30 ) = 0;

        for ( int i = 0; i < ANIMATION_LAYER_COUNT; i++ ) {
            const auto layer = player->anim_overlays( ) ? ( player->anim_overlays( ) + 1 ) : nullptr;

            if ( layer ) {
                *reinterpret_cast< int * >( reinterpret_cast< uintptr_t >( layer ) + 8 ) = 0;
                *reinterpret_cast< int * >( reinterpret_cast< uintptr_t >( layer ) + 12 ) = -1;
                *reinterpret_cast< int * >( reinterpret_cast< uintptr_t >( layer ) + 16 ) = -1;
            }
        }
    }

    state->m_flAnimDuckAmount = std::clamp< float >( valve_math::approach( std::clamp< float >( player->duck_amount( ) + state->m_flDuckAdditional, 0.f, 1.f ), state->m_flAnimDuckAmount, state->m_flLastUpdateIncrement * 6.0f ), 0, 1 );

    {
        g_interfaces.model_cache->begin_lock( );

        utils::get_method< void( __thiscall * )( void *, int ) >( player, 213 )( player, 0 );
        *reinterpret_cast< float * >( reinterpret_cast< uintptr_t >( player ) + 0xA18 ) = 0.0f;

        if ( *reinterpret_cast< float * >( reinterpret_cast< uintptr_t >( player ) + 0xA14 ) != 0.0f ) {
            *reinterpret_cast< float * >( reinterpret_cast< uintptr_t >( player ) + 0xA14 ) = 0;
            player->invalidate_physics_recursive( 8 );
        }

        g_interfaces.model_cache->end_lock( );
    }

    setup_velocity( state );
    setup_aim_matrix( state );
    setup_weapon_action( state );
    setup_movement( state );
    setup_alive_loop( state );
    setup_whole_body_action( state );
    setup_flashed_reaction( state );
    setup_flinch( state );
    setup_lean( state );

    for ( auto i = 0; i < ANIMATION_LAYER_COUNT; i++ ) {
        const auto layer = player->anim_overlays( ) ? ( player->anim_overlays( ) + i ) : nullptr;

        if ( layer && !layer->sequence )
            set_weight( state, i, 0.0f );
    }

    player->set_abs_angles( vector_3d{ 0, state->m_flFootYaw, 0 } );

    *enable_invalidate_bone_cache = true;

    state->m_pWeaponLast = state->m_pWeapon;
    state->m_vecPositionLast = state->m_vecPositionCurrent;
    state->m_bFirstRunSinceInit = false;
    state->m_flLastUpdateTime = g_interfaces.global_vars->curtime;
    state->m_nLastUpdateFrame = g_interfaces.global_vars->framecount;
}

void c_animation_state_rebuilt::increment_layer_weight( c_csgo_player_animstate *state, int layer_idx ) {
    auto layer = state->m_pPlayer->anim_overlays( ) + layer_idx;

    if ( !layer )
        return;

    if ( abs( layer->weight_delta_rate ) <= 0 )
        return;

    float flCurrentWeight = layer->weight;

    flCurrentWeight += state->m_flLastUpdateIncrement * layer->weight_delta_rate;
    flCurrentWeight = std::clamp< float >( flCurrentWeight, 0, 1 );

    set_weight( state, layer_idx, flCurrentWeight );
}

void c_animation_state_rebuilt::setup_whole_body_action( c_csgo_player_animstate *state ) {
    int layer_idx = ANIMATION_LAYER_WHOLE_BODY;

    if ( state->m_pPlayer->anim_overlays( )[ layer_idx ].weight > 0 ) {
        increment_layer_cycle( state, layer_idx, false );
        increment_layer_weight( state, layer_idx );
    }
}

bool m_bJumping;

void c_animation_state_rebuilt::setup_velocity( c_csgo_player_animstate *state ) {
    auto player = state->m_pPlayer;

    if ( !state || !player )
        return;

    static auto GetSeqDesc = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 79 04 00 75 25 8B 45 08 8B 09 85 C0 78 08 3B 81" ) ).get< void *( __thiscall * ) ( void *, int ) >( );

    g_interfaces.model_cache->begin_lock( );

    auto vecAbsVelocity = player->abs_velocity( );

    state->m_flVelocityLengthZ = vecAbsVelocity.z;
    if ( glm::length( vecAbsVelocity ) < 0.0001f )
        vecAbsVelocity = vector_3d( );
    vecAbsVelocity.z = 0;

    state->m_bPlayerIsAccelerating = ( math::length_sqr( state->m_vecVelocityLast ) < math::length_sqr( vecAbsVelocity ) );

    state->m_vecVelocity = valve_math::approach( vecAbsVelocity, state->m_vecVelocity, state->m_flLastUpdateIncrement * 2000.f );
    state->m_vecVelocityNormalized = math::normalize_angle( state->m_vecVelocity );

    state->m_flVelocityLengthXY = std::min< float >( glm::length( state->m_vecVelocity ), 260.f );

    if ( state->m_flVelocityLengthXY > 0.0f )
        state->m_vecVelocityNormalizedNonZero = state->m_vecVelocityNormalized;

    float flMaxSpeedRun = state->m_pWeapon ? std::max< float >( player->scoped( ) ? state->m_pWeapon->get_weapon_data( )->max_speed_alt : state->m_pWeapon->get_weapon_data( )->max_speed, 0.001f ) : CS_PLAYER_SPEED_RUN;

    state->m_flSpeedAsPortionOfRunTopSpeed = std::clamp< float >( state->m_flVelocityLengthXY / flMaxSpeedRun, 0.f, 1.f );
    state->m_flSpeedAsPortionOfWalkTopSpeed = state->m_flVelocityLengthXY / ( flMaxSpeedRun * CS_PLAYER_SPEED_WALK_MODIFIER );
    state->m_flSpeedAsPortionOfCrouchTopSpeed = state->m_flVelocityLengthXY / ( flMaxSpeedRun * CS_PLAYER_SPEED_DUCK_MODIFIER );

    if ( state->m_flSpeedAsPortionOfWalkTopSpeed >= 1 )
        state->m_flStaticApproachSpeed = state->m_flVelocityLengthXY;
    else if ( state->m_flSpeedAsPortionOfWalkTopSpeed < 0.5f )
        state->m_flStaticApproachSpeed = valve_math::approach( 80, state->m_flStaticApproachSpeed, state->m_flLastUpdateIncrement * 60 );

    bool bStartedMovingThisFrame = false;
    bool bStoppedMovingThisFrame = false;

    if ( state->m_flVelocityLengthXY > 0 ) {
        bStartedMovingThisFrame = ( state->m_flDurationMoving <= 0 );
        state->m_flDurationStill = 0;
        state->m_flDurationMoving += state->m_flLastUpdateIncrement;
    } else {
        bStoppedMovingThisFrame = ( state->m_flDurationStill <= 0 );
        state->m_flDurationMoving = 0;
        state->m_flDurationStill += state->m_flLastUpdateIncrement;
    }

    if ( !state->m_bAdjustStarted && bStoppedMovingThisFrame && state->m_bOnGround && !state->m_bOnLadder && !state->m_bLanding && state->m_flStutterStep < 50 ) {
        set_sequence( state, ANIMATION_LAYER_ADJUST, select_weighted_sequence( state, ACT_CSGO_IDLE_ADJUST_STOPPEDMOVING ) );
        state->m_bAdjustStarted = true;
    }

    if ( get_layer_activity( state, ANIMATION_LAYER_ADJUST ) == ACT_CSGO_IDLE_ADJUST_STOPPEDMOVING ||
         get_layer_activity( state, ANIMATION_LAYER_ADJUST ) == ACT_CSGO_IDLE_TURN_BALANCEADJUST ) {
        if ( state->m_bAdjustStarted && state->m_flSpeedAsPortionOfCrouchTopSpeed <= 0.25f ) {
            increment_layer_cycle_weight_rate_generic( state, ANIMATION_LAYER_ADJUST );
            state->m_bAdjustStarted = !( is_layer_sequence_completed( state, ANIMATION_LAYER_ADJUST ) );
        } else {
            state->m_bAdjustStarted = false;
            float flWeight = player->anim_overlays( )[ ANIMATION_LAYER_ADJUST ].weight;
            set_weight( state, ANIMATION_LAYER_ADJUST, valve_math::approach( 0, flWeight, state->m_flLastUpdateIncrement * 5 ) );
            set_weight_delta_rate( state, ANIMATION_LAYER_ADJUST, flWeight );
        }
    }

    state->m_flFootYawLast = state->m_flFootYaw;
    state->m_flFootYaw = std::clamp< float >( state->m_flFootYaw, -360.f, 360.f );
    float flEyeFootDelta = valve_math::angle_diff( state->m_flEyeYaw, state->m_flFootYaw );

    float flAimMatrixWidthRange = valve_math::Lerp( std::clamp< float >( state->m_flSpeedAsPortionOfWalkTopSpeed, 0.f, 1.f ), 1.0f, valve_math::Lerp( state->m_flWalkToRunTransition, CSGO_ANIM_AIM_NARROW_WALK, CSGO_ANIM_AIM_NARROW_RUN ) );

    if ( state->m_flAnimDuckAmount > 0 )
        flAimMatrixWidthRange = valve_math::Lerp( state->m_flAnimDuckAmount * std::clamp< float >( state->m_flSpeedAsPortionOfCrouchTopSpeed, 0.f, 1.f ), flAimMatrixWidthRange, CSGO_ANIM_AIM_NARROW_CROUCHMOVING );

    float flTempYawMax = state->m_flAimYawMax * flAimMatrixWidthRange;
    float flTempYawMin = state->m_flAimYawMin * flAimMatrixWidthRange;

    if ( flEyeFootDelta <= flTempYawMax ) {
        if ( flTempYawMin > flEyeFootDelta )
            state->m_flFootYaw = fabs( flTempYawMin ) + state->m_flEyeYaw;
    } else {
        state->m_flFootYaw = state->m_flEyeYaw - fabs( flTempYawMax );
    }

    state->m_flFootYaw = valve_math::angle_normalize( state->m_flFootYaw );

    float &m_flLowerBodyRealignTimer = state->m_flStaticApproachSpeed;

    if ( state->m_bOnGround ) {
        if ( state->m_flVelocityLengthXY > 0.1f ) {
            state->m_flFootYaw = valve_math::approach_angle( state->m_flEyeYaw, state->m_flFootYaw, state->m_flLastUpdateIncrement * ( 30.0f + 20.0f * state->m_flWalkToRunTransition ) );
        } else {
            state->m_flFootYaw = valve_math::approach_angle( player->lower_body_yaw_target( ), state->m_flFootYaw, state->m_flLastUpdateIncrement * CSGO_ANIM_LOWER_CATCHUP_IDLE );

            if ( g_interfaces.global_vars->curtime > m_flLowerBodyRealignTimer && abs( valve_math::angle_diff( state->m_flFootYaw, state->m_flEyeYaw ) ) > 35.0f ) {
            }
        }
    }

    if ( state->m_flVelocityLengthXY <= CS_PLAYER_SPEED_STOPPED && state->m_bOnGround && !state->m_bOnLadder && !state->m_bLanding && state->m_flLastUpdateIncrement > 0 && abs( valve_math::angle_diff( state->m_flFootYawLast, state->m_flFootYaw ) / state->m_flLastUpdateIncrement > CSGO_ANIM_READJUST_THRESHOLD ) ) {
        set_sequence( state, ANIMATION_LAYER_ADJUST, select_weighted_sequence( state, ACT_CSGO_IDLE_TURN_BALANCEADJUST ) );
        state->m_bAdjustStarted = true;
    }

    if ( state->m_flVelocityLengthXY > 0 && state->m_bOnGround ) {
        float flRawYawIdeal = ( atan2( -state->m_vecVelocity[ 1 ], -state->m_vecVelocity[ 0 ] ) * 180 / glm::pi< float >( ) );

        if ( flRawYawIdeal < 0 )
            flRawYawIdeal += 360;

        state->m_flMoveYawIdeal = valve_math::angle_normalize( valve_math::angle_diff( flRawYawIdeal, state->m_flFootYaw ) );
    }

    state->m_flMoveYawCurrentToIdeal = valve_math::angle_normalize( valve_math::angle_diff( state->m_flMoveYawIdeal, state->m_flMoveYaw ) );

    if ( bStartedMovingThisFrame && state->m_flMoveWeight <= 0 ) {
        state->m_flMoveYaw = state->m_flMoveYawIdeal;

        int nMoveSeq = player->anim_overlays( )[ ANIMATION_LAYER_MOVEMENT_MOVE ].sequence;

        if ( nMoveSeq != -1 ) {
            void *seqdesc = GetSeqDesc( player->get_model_ptr( ), nMoveSeq );

            if ( *reinterpret_cast< int * >( reinterpret_cast< uintptr_t >( seqdesc ) + 0x31 ) > 0 ) {
                if ( abs( valve_math::angle_diff( state->m_flMoveYaw, 180 ) ) <= EIGHT_WAY_WIDTH ) {
                    state->m_flPrimaryCycle = player->get_first_sequence_anim_tag( nMoveSeq, ANIMTAG_STARTCYCLE_N, 0, 1 );
                } else if ( abs( valve_math::angle_diff( state->m_flMoveYaw, 135 ) ) <= EIGHT_WAY_WIDTH ) {
                    state->m_flPrimaryCycle = player->get_first_sequence_anim_tag( nMoveSeq, ANIMTAG_STARTCYCLE_NE, 0, 1 );
                } else if ( abs( valve_math::angle_diff( state->m_flMoveYaw, 90 ) ) <= EIGHT_WAY_WIDTH ) {
                    state->m_flPrimaryCycle = player->get_first_sequence_anim_tag( nMoveSeq, ANIMTAG_STARTCYCLE_E, 0, 1 );
                } else if ( abs( valve_math::angle_diff( state->m_flMoveYaw, 45 ) ) <= EIGHT_WAY_WIDTH ) {
                    state->m_flPrimaryCycle = player->get_first_sequence_anim_tag( nMoveSeq, ANIMTAG_STARTCYCLE_SE, 0, 1 );
                } else if ( abs( valve_math::angle_diff( state->m_flMoveYaw, 0 ) ) <= EIGHT_WAY_WIDTH ) {
                    state->m_flPrimaryCycle = player->get_first_sequence_anim_tag( nMoveSeq, ANIMTAG_STARTCYCLE_S, 0, 1 );
                } else if ( abs( valve_math::angle_diff( state->m_flMoveYaw, -45 ) ) <= EIGHT_WAY_WIDTH ) {
                    state->m_flPrimaryCycle = player->get_first_sequence_anim_tag( nMoveSeq, ANIMTAG_STARTCYCLE_SW, 0, 1 );
                } else if ( abs( valve_math::angle_diff( state->m_flMoveYaw, -90 ) ) <= EIGHT_WAY_WIDTH ) {
                    state->m_flPrimaryCycle = player->get_first_sequence_anim_tag( nMoveSeq, ANIMTAG_STARTCYCLE_W, 0, 1 );
                } else if ( abs( valve_math::angle_diff( state->m_flMoveYaw, -135 ) ) <= EIGHT_WAY_WIDTH ) {
                    state->m_flPrimaryCycle = player->get_first_sequence_anim_tag( nMoveSeq, ANIMTAG_STARTCYCLE_NW, 0, 1 );
                }
            }
        }
    } else {
        if ( player->anim_overlays( )[ ANIMATION_LAYER_MOVEMENT_STRAFECHANGE ].weight >= 1 ) {
            state->m_flMoveYaw = state->m_flMoveYawIdeal;
        } else {
            float flMoveWeight = valve_math::Lerp( state->m_flAnimDuckAmount, std::clamp< float >( state->m_flSpeedAsPortionOfWalkTopSpeed, 0, 1 ), std::clamp< float >( state->m_flSpeedAsPortionOfCrouchTopSpeed, 0, 1 ) );
            float flRatio = valve_math::bias( flMoveWeight, 0.18f ) + 0.1f;

            state->m_flMoveYaw = valve_math::angle_normalize( state->m_flMoveYaw + ( state->m_flMoveYawCurrentToIdeal * flRatio ) );
        }
    }

    state->m_tPoseParamMappings[ PLAYER_POSE_PARAM_MOVE_YAW ].SetValue( player, state->m_flMoveYaw );

    float flAimYaw = valve_math::angle_diff( state->m_flEyeYaw, state->m_flFootYaw );

    if ( flAimYaw >= 0 && state->m_flAimYawMax != 0 )
        flAimYaw = ( flAimYaw / state->m_flAimYawMax ) * 60.0f;
    else if ( state->m_flAimYawMin != 0 )
        flAimYaw = ( flAimYaw / state->m_flAimYawMin ) * -60.0f;

    state->m_tPoseParamMappings[ PLAYER_POSE_PARAM_BODY_YAW ].SetValue( player, flAimYaw );

    float flPitch = valve_math::angle_diff( state->m_flEyePitch, 0 );

    if ( flPitch > 0 ) {
        flPitch = ( flPitch / state->m_flAimPitchMax ) * CSGO_ANIM_AIMMATRIX_DEFAULT_PITCH_MAX;
    } else {
        flPitch = ( flPitch / state->m_flAimPitchMin ) * CSGO_ANIM_AIMMATRIX_DEFAULT_PITCH_MIN;
    }

    state->m_tPoseParamMappings[ PLAYER_POSE_PARAM_BODY_PITCH ].SetValue( player, flPitch );
    state->m_tPoseParamMappings[ PLAYER_POSE_PARAM_SPEED ].SetValue( player, state->m_flSpeedAsPortionOfWalkTopSpeed );
    state->m_tPoseParamMappings[ PLAYER_POSE_PARAM_STAND ].SetValue( player, 1.0f - ( state->m_flAnimDuckAmount * state->m_flInAirSmoothValue ) );
}
#pragma optimize( "", on )

#define CSGO_ANIm_flWalkToRunTransition_SPEED 2.0f
#define CSGO_ANIM_ONGROUND_FUZZY_APPROACH 8.0f
#define CSGO_ANIM_ONGROUND_FUZZY_APPROACH_CROUCH 16.0f
#define CSGO_ANIM_LADDER_CLIMB_COVERAGE 100.0f
#define CSGO_ANIM_RUN_ANIM_PLAYBACK_MULTIPLIER 0.85f
#define MAX_ANIMSTATE_ANIMNAME_CHARS 64
#define ANIM_TRANSITION_WALK_TO_RUN 0
#define ANIM_TRANSITION_RUN_TO_WALK 1

void c_animation_state_rebuilt::update_anim_layer( c_csgo_player_animstate *state, int layer_idx, int seq, float playback_rate, float weight, float cycle ) {
    static auto UpdateLayerOrderPreset = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 51 53 56 57 8B F9 83 7F 60 00 0F 84 ? ? ? ? 83" ) ).get< void( __thiscall * )( c_csgo_player_animstate *, int, int ) >( );

    if ( seq > 1 ) {
        g_interfaces.model_cache->begin_lock( );

        const auto layer = state->m_pPlayer->anim_overlays( ) + layer_idx;

        if ( !layer )
            return;

        layer->sequence = seq;
        layer->playback_rate = playback_rate;

        set_cycle( state, layer_idx, std::clamp< float >( cycle, 0, 1 ) );
        set_weight( state, layer_idx, std::clamp< float >( weight, 0, 1 ) );
        UpdateLayerOrderPreset( state, layer_idx, seq );

        g_interfaces.model_cache->end_lock( );
    }
}

void c_animation_state_rebuilt::trigger_animation_events( c_csgo_player_animstate *state ) {
    auto player = state->m_pPlayer;

    if ( !player || player != globals::local_player )
        return;

    const bool on_ground = ( globals::local_player->flags( ) & player_flags::on_ground ) != 0;
    const bool landed_on_ground_this_frame = on_ground && !state->m_bOnGround;

    bool &m_bJumping = state->m_bFlashed;

    if ( state->m_pPlayer == globals::local_player && !on_ground && state->m_bOnGround && state->m_pPlayer->velocity( ).z > 0.0f ) {
        m_bJumping = true;
        set_sequence( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, select_weighted_sequence( state, ACT_CSGO_JUMP ) );
    }

    const bool stopped_moving_this_frame = state->m_flVelocityLengthXY <= 0.0f && state->m_flDurationStill <= 0.0f;
    const bool previously_on_ladder = state->m_bOnLadder;
    const bool on_ladder = state->m_pPlayer->move_type( ) == move_types::ladder;
    const bool started_laddering_this_frame = ( !previously_on_ladder && on_ladder );
    const bool stopped_laddering_this_frame = ( previously_on_ladder && !on_ladder );

    if ( !state->m_bAdjustStarted && stopped_moving_this_frame && on_ground && !on_ladder && !state->m_bLanding && state->m_flStutterStep < 50.0f ) {
        set_sequence( state, ANIMATION_LAYER_ADJUST, select_weighted_sequence( state, 980 ) );
        state->m_bAdjustStarted = true;
    }

    const int adjust_activity = get_layer_activity( state, ANIMATION_LAYER_ADJUST );

    if ( adjust_activity == 980 || adjust_activity == 979 ) {
        if ( state->m_bAdjustStarted && state->m_flSpeedAsPortionOfCrouchTopSpeed <= 0.25f ) {
            increment_layer_cycle_weight_rate_generic( state, ANIMATION_LAYER_ADJUST );
            state->m_bAdjustStarted = !is_layer_sequence_completed( state, ANIMATION_LAYER_ADJUST );
        } else {
            state->m_bAdjustStarted = false;

            auto &layer = state->m_pPlayer->anim_overlays( )[ ANIMATION_LAYER_ADJUST ];
            const float weight = layer.weight;
            layer.weight = valve_math::approach( 0.0f, weight, state->m_flLastUpdateIncrement * 5.0f );
            set_weight_delta_rate( state, ANIMATION_LAYER_ADJUST, weight );
        }
    }

    auto weapon = g_interfaces.entity_list->get_client_entity_from_handle< c_cs_weapon_base * >( state->m_pPlayer->weapon_handle( ) );

    if ( !weapon )
        return;

    const auto max_speed_run = weapon ? std::max( weapon->get_weapon_data( )->max_speed, 0.001f ) : CS_PLAYER_SPEED_RUN;
    const auto velocity = state->m_pPlayer->velocity( );

    state->m_flVelocityLengthXY = std::min< float >( glm::length( velocity ), CS_PLAYER_SPEED_RUN );

    if ( state->m_flVelocityLengthXY > 0.0f )
        state->m_vecVelocityNormalizedNonZero = math::normalize_angle( velocity );

    state->m_flSpeedAsPortionOfWalkTopSpeed = state->m_flVelocityLengthXY / ( max_speed_run * CS_PLAYER_SPEED_WALK_MODIFIER );

    if ( globals::local_player && state->m_pPlayer == globals::local_player ) {
        const uint32_t buttons = *reinterpret_cast< uint32_t * >( reinterpret_cast< uintptr_t >( state->m_pPlayer ) + 0x3208 );

        bool move_right = ( buttons & ( buttons::move_right ) ) != 0;
        bool move_left = ( buttons & ( buttons::move_left ) ) != 0;
        bool move_forward = ( buttons & ( buttons::forward ) ) != 0;
        bool move_backward = ( buttons & ( buttons::back ) ) != 0;

        vector_3d forward, right;
        valve_math::angle_vectors( vector_3d( 0, state->m_flFootYaw, 0 ), &forward, &right, nullptr );

        auto right_normalized = math::normalize_angle( right );

        const float vel_to_right_dot = glm::dot( state->m_vecVelocityNormalizedNonZero, right_normalized );
        const float vel_to_forward_dot = glm::dot( state->m_vecVelocityNormalizedNonZero, forward );

        const bool strafe_right = ( state->m_flSpeedAsPortionOfWalkTopSpeed >= 0.73f && move_right && !move_left && vel_to_right_dot < -0.63f );
        const bool strafe_left = ( state->m_flSpeedAsPortionOfWalkTopSpeed >= 0.73f && move_left && !move_right && vel_to_right_dot > 0.63f );
        const bool strafe_forward = ( state->m_flSpeedAsPortionOfWalkTopSpeed >= 0.65f && move_forward && !move_backward && vel_to_forward_dot < -0.55f );
        const bool strafe_back = ( state->m_flSpeedAsPortionOfWalkTopSpeed >= 0.65f && move_backward && !move_forward && vel_to_forward_dot > 0.55f );

        *reinterpret_cast< bool * >( reinterpret_cast< uintptr_t >( state->m_pPlayer ) + 0x39E0 ) = ( strafe_right || strafe_left || strafe_forward || strafe_back );

        if ( ( state->m_flLadderWeight > 0.0f || on_ladder ) && started_laddering_this_frame )
            set_sequence( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, select_weighted_sequence( state, 987 ) );

        if ( on_ground ) {
            bool next_landing = false;
            if ( !state->m_bLanding && ( landed_on_ground_this_frame || stopped_laddering_this_frame ) ) {
                set_sequence( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, select_weighted_sequence( state, ( state->m_flDurationInAir > 1.0f ) ? 989 : 988 ) );
                next_landing = true;
            }

            if ( next_landing && get_layer_activity( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ) != 987 ) {
                m_bJumping = false;

                auto &layer = state->m_pPlayer->anim_overlays( )[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ];

                const float backup_cycle = layer.cycle;

                increment_layer_cycle( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, false );

                if ( is_layer_sequence_completed( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ) )
                    next_landing = false;

                layer.cycle = backup_cycle;
            }

            if ( !next_landing && !m_bJumping && state->m_flLadderWeight <= 0.0f ) {
                auto &layer = state->m_pPlayer->anim_overlays( )[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ];
                layer.weight = 0.0f;
            }
        } else if ( !on_ladder ) {
            if ( !state->m_bOnGround || stopped_laddering_this_frame ) {
                if ( m_bJumping ) {
                    set_sequence( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, select_weighted_sequence( state, ACT_CSGO_FALL ) );
                }
            }
            increment_layer_cycle( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, false );
        }
    }
}

void c_animation_state_rebuilt::setup_alive_loop( c_csgo_player_animstate *state ) {
    if ( !state || !state->m_pPlayer )
        return;

    auto anim_layers = state->m_pPlayer->anim_overlays( );

    if ( get_layer_activity( state, ANIMATION_LAYER_ALIVELOOP ) == 981 ) {
        if ( state->m_pWeapon && state->m_pWeapon != state->m_pWeaponLast ) {
            float old_cycle = anim_layers[ ANIMATION_LAYER_ALIVELOOP ].cycle;

            set_sequence( state, ANIMATION_LAYER_ALIVELOOP, select_weighted_sequence( state, 981 ) );

            anim_layers[ ANIMATION_LAYER_ALIVELOOP ].cycle = old_cycle;
        } else if ( is_layer_sequence_completed( state, ANIMATION_LAYER_ALIVELOOP ) ) {
            anim_layers[ ANIMATION_LAYER_ALIVELOOP ].playback_rate = state->m_pPlayer->get_sequence_cycle_rate( anim_layers[ ANIMATION_LAYER_ALIVELOOP ].sequence ) * math::random_float( 0.8f, 1.1f );
        } else {
            anim_layers[ ANIMATION_LAYER_ALIVELOOP ].weight = valve_math::remap_val_clamped( state->m_flSpeedAsPortionOfRunTopSpeed, 0.55f, 0.9f, 1.f, 0.f );
        }
    } else {
        set_sequence( state, ANIMATION_LAYER_ALIVELOOP, select_weighted_sequence( state, 981 ) );
        anim_layers[ ANIMATION_LAYER_ALIVELOOP ].cycle = math::random_float( 0.f, 1.f );
        anim_layers[ ANIMATION_LAYER_ALIVELOOP ].playback_rate = state->m_pPlayer->get_sequence_cycle_rate( anim_layers[ ANIMATION_LAYER_ALIVELOOP ].sequence ) * math::random_float( 0.8f, 1.1f );
    }
}

void c_animation_state_rebuilt::setup_flashed_reaction( c_csgo_player_animstate *state ) {
    static auto SetupFlashedReaction = signature::find( _xs( "client.dll" ), _xs( "51 8B 41 60 83 B8 ?? ?? ?? ?? ?? 74 3E 8B 90 ?? ?? ?? ?? 81 C2" ) ).get< void( __thiscall * )( c_csgo_player_animstate * ) >( );
    SetupFlashedReaction( state );
}

void c_animation_state_rebuilt::setup_flinch( c_csgo_player_animstate *state ) {
    static auto SetupFlinch = signature::find( _xs( "client.dll" ), _xs( "E8 ? ? ? ? 8B CF E8 ? ? ? ? 33 C0 89 44 24" ) ).add( 0x1 ).rel32( ).get< void( __thiscall * )( c_csgo_player_animstate * ) >( );
    SetupFlinch( state );
}

#pragma optimize( "", off )
void c_animation_state_rebuilt::setup_aim_matrix( c_csgo_player_animstate *state ) {
    static auto SetupAimMatrix = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 81 EC ? ? ? ? 53 56 57 8B 3D ? ? ? ? 8B" ) ).get< void( __thiscall * )( c_csgo_player_animstate * ) >( );
    SetupAimMatrix( state );
}
#pragma optimize( "", on )

void c_animation_state_rebuilt::setup_weapon_action( c_csgo_player_animstate *state ) {
    static auto SetupWeaponAction = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 81 EC ? ? ? ? 53 56 57 8B 3D ? ? ? ? 8B" ) ).get< void( __thiscall * )( c_csgo_player_animstate * ) >( );
    SetupWeaponAction( state );
}

void c_animation_state_rebuilt::setup_lean( c_csgo_player_animstate *state ) {
    static auto SetupLean = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 E4 F8 A1 ? ? ? ? 83 EC 20 F3 0F 10 48 ? 56 57 8B F9" ) ).get< void( __thiscall * )( c_csgo_player_animstate * ) >( );
    SetupLean( state );
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
    static auto CCSGOPlayerAnimState_GetLayerIdealWeightFromSeqCycle = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 EC 08 53 56 8B 35 ? ? ? ? 57 8B F9 8B CE 8B 06 FF 90" ) ).get< float( __thiscall * )( c_csgo_player_animstate *, int ) >( );
    return CCSGOPlayerAnimState_GetLayerIdealWeightFromSeqCycle( state, layer_idx );
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
    static auto CCSGOPlayerAnimState_get_layer_activity = signature::find( _xs( "client.dll" ), _xs( "51 53 56 8B 35 ? ? ? ? 57 8B F9 8B CE 8B 06 FF 90 ? ? ? ? 8B 7F 60 83 BF" ) ).get< int( __thiscall * )( c_csgo_player_animstate *, int ) >( );
    return CCSGOPlayerAnimState_get_layer_activity( state, layer_idx );
}