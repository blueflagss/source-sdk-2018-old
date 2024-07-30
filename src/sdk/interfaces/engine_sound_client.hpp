#pragma once
enum soundlevel_t {
    SNDLVL_NONE = 0,

    SNDLVL_20dB = 20,// rustling leaves
    SNDLVL_25dB = 25,// whispering
    SNDLVL_30dB = 30,// library
    SNDLVL_35dB = 35,
    SNDLVL_40dB = 40,
    SNDLVL_45dB = 45,// refrigerator

    SNDLVL_50dB = 50,// 3.9	// average home
    SNDLVL_55dB = 55,// 3.0

    SNDLVL_IDLE = 60,// 2.0
    SNDLVL_60dB = 60,// 2.0	// normal conversation, clothes dryer

    SNDLVL_65dB = 65,  // 1.5	// washing machine, dishwasher
    SNDLVL_STATIC = 66,// 1.25

    SNDLVL_70dB = 70,// 1.0	// car, vacuum cleaner, mixer, electric sewing machine

    SNDLVL_NORM = 75,
    SNDLVL_75dB = 75,// 0.8	// busy traffic

    SNDLVL_80dB = 80,   // 0.7	// mini-bike, alarm clock, noisy restaurant, office tabulator, outboard motor, passing snowmobile
    SNDLVL_TALKING = 80,// 0.7
    SNDLVL_85dB = 85,   // 0.6	// average factory, electric shaver
    SNDLVL_90dB = 90,   // 0.5	// screaming child, passing motorcycle, convertible ride on frw
    SNDLVL_95dB = 95,
    SNDLVL_100dB = 100,// 0.4	// subway train, diesel truck, woodworking shop, pneumatic drill, boiler shop, jackhammer
    SNDLVL_105dB = 105,// helicopter, power mower
    SNDLVL_110dB = 110,// snowmobile drvrs seat, inboard motorboat, sandblasting
    SNDLVL_120dB = 120,// auto horn, propeller aircraft
    SNDLVL_130dB = 130,// air raid siren

    SNDLVL_GUNFIRE = 140,// 0.27	// THRESHOLD OF PAIN, gunshot, jet engine
    SNDLVL_140dB = 140,  // 0.2

    SNDLVL_150dB = 150,// 0.2

    SNDLVL_180dB = 180,// rocket launching

    // NOTE: Valid soundlevel_t values are 0-255.
    //       256-511 are reserved for sounds using goldsrc compatibility attenuation.
};

struct soundinfo_t {
    int nSequenceNumber;
    int nEntityIndex;
    int nChannel;
    const char *pszName;// UNDONE: Make this a FilenameHandle_t to avoid bugs with arrays of these
    vector_3d vOrigin;
    vector_3d vDirection;
    float fVolume;
    soundlevel_t Soundlevel;
    bool bLooping;
    int nPitch;
    int nSpecialDSP;
    vector_3d vListenerOrigin;
    int nFlags;
    int nSoundNum;
    float fDelay;
    bool bIsSentence;
    bool bIsAmbient;
    int nSpeakerEntity;
};

    struct sndinfo_t {
    // Sound Guid
    int m_nGuid;
    uint32_t m_filenameHandle;// filesystem filename handle - call IFilesystem to conver this to a string
    int m_nSoundSource;
    int m_nChannel;
    // If a sound is being played through a speaker entity (e.g., on a monitor,), this is the
    //  entity upon which to show the lips moving, if the sound has sentence data
    int m_nSpeakerEntity;
    float m_flVolume;
    float m_flLastSpatializedVolume;
    // Radius of this sound effect (spatialization is different within the radius)
    float m_flRadius;
    int m_nPitch;
    vector_3d *m_pOrigin;
    vector_3d *m_pDirection;

    // if true, assume sound source can move and update according to entity
    bool m_bUpdatePositions;
    // true if playing linked sentence
    bool m_bIsSentence;
    // if true, bypass all dsp processing for this sound (ie: music)
    bool m_bDryMix;
    // true if sound is playing through in-game speaker entity.
    bool m_bSpeaker;
    // for snd_show, networked sounds get colored differently than local sounds
    bool m_bFromServer;
};

class c_engine_sound_client {
public:
    void get_active_sounds( c_utl_vector< sndinfo_t > &sndlist ) {
        return utils::get_method< void( __thiscall * )( void *, c_utl_vector< sndinfo_t > & ) >( this, 18 )( this, sndlist );
    }
};