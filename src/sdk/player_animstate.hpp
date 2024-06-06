#pragma once

enum AnimationLayer_t {
    ANIMATION_LAYER_AIMMATRIX = 0,
    ANIMATION_LAYER_WEAPON_ACTION,
    ANIMATION_LAYER_WEAPON_ACTION_RECROUCH,
    ANIMATION_LAYER_ADJUST,
    ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL,
    ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB,
    ANIMATION_LAYER_MOVEMENT_MOVE,
    ANIMATION_LAYER_MOVEMENT_STRAFECHANGE,
    ANIMATION_LAYER_WHOLE_BODY,
    ANIMATION_LAYER_FLASHED,
    ANIMATION_LAYER_FLINCH,
    ANIMATION_LAYER_ALIVELOOP,
    ANIMATION_LAYER_LEAN,
    ANIMATION_LAYER_COUNT,
};

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

struct animstate_pose_param_cache_t {
    bool m_bInitialized;
    int m_nIndex;
    const char *m_szName;

    bool Init( c_cs_player *pPlayer, const char *szPoseParamName ) {
        g_interfaces.model_cache->begin_lock( );
        m_szName = szPoseParamName;
        m_nIndex = pPlayer->lookup_pose_parameter( szPoseParamName );

        if ( m_nIndex != -1 ) {
            m_bInitialized = true;
        }

        g_interfaces.model_cache->end_lock( );

        return m_bInitialized;
    }

    float GetValue( c_cs_player *pPlayer ) {
        if ( !m_bInitialized ) {
            Init( pPlayer, m_szName );
        }

        if ( m_bInitialized && pPlayer ) {
            return pPlayer->get_pose_parameter( m_nIndex );
        }

        return 0;
    }

    void SetValue( c_cs_player *pPlayer, float flValue ) {
        if ( !m_bInitialized ) {
            Init( pPlayer, m_szName );
        }
        if ( m_bInitialized && pPlayer ) {
            g_interfaces.model_cache->begin_lock( );
            pPlayer->set_pose_parameter( m_nIndex, flValue );
            g_interfaces.model_cache->end_lock( );
        }
    }
};

class c_csgo_player_animstate {
public:
    const int *m_pLayerOrderPreset;
    bool m_bFirstRunSinceInit;
    bool m_bFirstFootPlantSinceInit;
    int m_iLastUpdateFrame;
    float m_flEyePositionSmoothLerp;
    float m_flStrafeChangeWeightSmoothFalloff;
    PAD( 60 );
    int m_cachedModelIndex;
    float m_flStepHeightLeft;
    float m_flStepHeightRight;
    c_cs_weapon_base *m_pWeaponLastBoneSetup;
    c_cs_player *m_pPlayer;
    c_cs_weapon_base *m_pWeapon;
    c_cs_weapon_base *m_pWeaponLast;
    float m_flLastUpdateTime;
    int m_nLastUpdateFrame;
    float m_flLastUpdateIncrement;
    float m_flEyeYaw;
    float m_flEyePitch;
    float m_flFootYaw;
    float m_flFootYawLast;
    float m_flMoveYaw;
    float m_flMoveYawIdeal;
    float m_flMoveYawCurrentToIdeal;
    float m_flTimeToAlignLowerBody;
    float m_flPrimaryCycle;
    float m_flMoveWeight;
    float m_flMoveWeightSmoothed;
    float m_flAnimDuckAmount;
    float m_flDuckAdditional;
    float m_flRecrouchWeight;
    vector_3d m_vecPositionCurrent;
    vector_3d m_vecPositionLast;
    vector_3d m_vecVelocity;
    vector_3d m_vecVelocityNormalized;
    vector_3d m_vecVelocityNormalizedNonZero;
    float m_flVelocityLengthXY;
    float m_flVelocityLengthZ;
    float m_flSpeedAsPortionOfRunTopSpeed;
    float m_flSpeedAsPortionOfWalkTopSpeed;
    float m_flSpeedAsPortionOfCrouchTopSpeed;
    float m_flDurationMoving;
    float m_flDurationStill;
    bool m_bOnGround;
    bool m_bLanding;
    float m_flJumpToFall;
    float m_flDurationInAir;
    float m_flLeftGroundHeight;
    float m_flLandAnimMultiplier;
    float m_flWalkToRunTransition;
    bool m_bLandedOnGroundThisFrame;
    bool m_bLeftTheGroundThisFrame;
    float m_flInAirSmoothValue;
    bool m_bOnLadder;
    float m_flLadderWeight;
    float m_flLadderSpeed;
    bool m_bWalkToRunTransitionState;
    bool m_bDefuseStarted;
    bool m_bPlantAnimStarted;
    bool m_bTwitchAnimStarted;
    bool m_bAdjustStarted;
    PAD( 20 );
    float m_flNextTwitchTime;
    float m_flTimeOfLastKnownInjury;
    float m_flLastVelocityTestTime;
    vector_3d m_vecVelocityLast;
    vector_3d m_vecTargetAcceleration;
    vector_3d m_vecAcceleration;
    float m_flAccelerationWeight;
    float m_flAimMatrixTransition;
    float m_flAimMatrixTransitionDelay;
    bool m_bFlashed;
    float m_flStrafeChangeWeight;
    float m_flStrafeChangeTargetWeight;
    float m_flStrafeChangeCycle;
    int m_nStrafeSequence;
    bool m_bStrafeChanging;
    float m_flDurationStrafing;
    float m_flFootLerp;
    bool m_bFeetCrossed;
    bool m_bPlayerIsAccelerating;
    animstate_pose_param_cache_t m_tPoseParamMappings[ 20 ];
    float m_flDurationMoveWeightIsTooHigh;
    float m_flStaticApproachSpeed;
    int m_nPreviousMoveState;
    float m_flStutterStep;
    float m_flActionWeightBiasRemainder;
    PAD( 112 );
    float m_flCameraSmoothHeight;
    bool m_bSmoothHeightValid;
    float m_flLastTimeVelocityOverTen;
    float m_flAimYawMin;
    float m_flAimYawMax;
    float m_flAimPitchMin;
    float m_flAimPitchMax;
    int m_nAnimstateModelVersion;

    void update( const vector_3d &angle ) {
        static auto update = signature::find( XOR( "client.dll" ), XOR( "E8 ? ? ? ? 0F 57 C0 0F 11 86" ) ).add( 0x1 ).rel32( ).get< void( __vectorcall * )( void *, void *, float, float, float, void * ) >( );

        if ( !update )
            return;

        update( this, nullptr, angle.z, angle.y, angle.x, nullptr );
    }

    const char *get_weapon_prefix( ) {
        static auto get_weapon_prefix = signature::find( "client.dll", "53 56 8B F1 57 33 FF 8B 4E 60 8B 01 FF 90 ? ? ? ? 89 46 64 85" ).get< const char *( __thiscall * ) ( void * ) >( );

        return get_weapon_prefix( this );
    }
};