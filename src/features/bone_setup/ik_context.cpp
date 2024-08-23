#include "ik_context.hpp"

c_ik_context::c_ik_context( ) {
    using ConstructFn = c_ik_context *( __thiscall * ) ( c_ik_context * );
    static auto Construct = signature::find( _xs( "client.dll" ), _xs( "56 8B F1 6A 00 6A 00 C7 86 ? ? ? ? ? ? ? ? 89 B6 ? ? ? ? C7 86" ) ).get< ConstructFn >( );
    Construct( this );
}

void c_ik_context::init( const c_studio_hdr *hdr, const vector_3d &local_angles, const vector_3d &local_origin, float current_time, int frame_count, int bone_mask ) {
    using InitFn = void( __thiscall * )( void *, const c_studio_hdr *, const vector_3d &, const vector_3d &, float, int, int );
    static auto init = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 EC 08 8B 45 08 56 57 8B F9 8D 8F ? ? ? ? 89 7D" ) ).get< InitFn >( );
    init( this, hdr, local_angles, local_origin, current_time, frame_count, bone_mask );
}

void c_ik_context::update_targets( vector_3d pos[], vector_4d qua[], matrix_3x4_aligned *matrix, uint8_t *boneComputed ) {
    using UpdateTargetsFn = void( __thiscall * )( void *, vector_3d [], vector_4d [], matrix_3x4_aligned *, uint8_t * );
    static auto update_targets = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 E4 F0 81 ? ? ? ? ? 33 D2 89" ) ).get< UpdateTargetsFn >( );
    update_targets( this, pos, qua, matrix, boneComputed );
}

void c_ik_context::solve_dependencies( vector_3d pos[], vector_4d qua[], matrix_3x4_aligned *matrix, uint8_t *boneComputed ) {
    using SolveDependenciesFn = void( __thiscall * )( void *, vector_3d[], vector_4d[], matrix_3x4_aligned *, uint8_t * );
    static auto solve_dependencies = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 E4 F0 81 EC ? ? ? ? 8B 81" ) ).get< SolveDependenciesFn >( );
    solve_dependencies( this, pos, qua, matrix, boneComputed );
}

struct CIKTarget {
    int m_iFrameCount;

private:
    char pad_00004[ 0x51 ];
};

void c_ik_context::clear_targets( ) {
    int v49 = 0;

    if ( *( int * ) ( std::uintptr_t( this ) + 4080 ) > 0 ) {
        int *iFramecounter = ( int * ) ( std::uintptr_t( this ) + 0xD0 );
        do {
            *iFramecounter = -9999;
            iFramecounter += 85;
            ++v49;
        } while ( v49 < *( int * ) ( std::uintptr_t( this ) + 0xFF0 ) );
    }
}

void c_ik_context::AddDependencies( void *seqdesc, int iSequence, float flCycle, const float poseParameters[], float flWeight ) {
    using AddDependenciesFn = void( __thiscall * )( void *, void *, int, float, const float[], float );
    static auto AddDependencies = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 81 EC ? ? ? ? 53 56 57 8B F9 0F 28 CB" ) ).get< uintptr_t >( );

    __asm
    {
			mov ecx, this
			movss xmm3, flCycle
			push flWeight
			push poseParameters
			push iSequence
			push seqdesc
			call AddDependencies
    }
}

void c_ik_context::CopyTo( c_ik_context *other, const unsigned short *iRemapping ) {
    using CopyToFn = void( __thiscall * )( void *, c_ik_context *, const unsigned short * );
    static auto CopyTo = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 EC ? 8B 45 ? 57 8B F9 89 7D ?") ).get< CopyToFn >( );
    CopyTo( this, other, iRemapping );
}