#pragma once
#include <globals.hpp>

class c_ik_context {
    PAD( 0x1070 );

public:
    c_ik_context( );

    void init( const c_studio_hdr *hdr, const vector_3d &local_angles, const vector_3d &local_origin, float current_time, int frame_count, int bone_mask );
    void update_targets( vector_3d pos[], vector_4d qua[], matrix_3x4_aligned *matrix, uint8_t *boneComputed );
    void solve_dependencies( vector_3d pos[], vector_4d qua[], matrix_3x4_aligned *matrix, uint8_t *boneComputed );
    void clear_targets( );
    void AddDependencies( void *seqdesc, int iSequence, float flCycle, const float poseParameters[], float flWeight );
    void CopyTo( c_ik_context *other, const unsigned short *iRemapping );
};