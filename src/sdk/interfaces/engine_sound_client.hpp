#pragma once

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