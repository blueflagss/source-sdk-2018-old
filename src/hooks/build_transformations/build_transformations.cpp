#include "build_transformations.hpp"

void __fastcall hooks::build_transformations::hook( REGISTERS, c_studio_hdr *hdr, const vector_3d &pos, void *quaternion, matrix_3x4_aligned const &matrix, uint32_t mask, bool *computed ) {
    const auto entity = reinterpret_cast< c_cs_player * >( reinterpret_cast< uintptr_t >( ecx ) - 4 );

    if ( !hdr )
        return reinterpret_cast< decltype( &hooks::build_transformations::hook ) >( hooks::build_transformations::original.trampoline( ).address( ) )( REGISTERS_OUT, hdr, pos, quaternion, matrix, mask, computed );

    auto studio_hdr = hdr->studio_hdr;

    if ( !studio_hdr )
        return reinterpret_cast< decltype( &hooks::build_transformations::hook ) >( hooks::build_transformations::original.trampoline( ).address( ) )( REGISTERS_OUT, hdr, pos, quaternion, matrix, mask, computed );

    const auto backup_bone_flags = studio_hdr->flags;

    for ( auto i = 0; i < studio_hdr->numbones; i++ ) {
        auto bone = studio_hdr->bone( i );

        if ( bone )
            bone->flags &= ~0x04;
    }

    const auto backup_is_jiggle_bones_enabled = entity->is_jiggle_bones_enabled( );

    entity->is_jiggle_bones_enabled( ) = false;
    reinterpret_cast< decltype( &hooks::build_transformations::hook ) >( hooks::build_transformations::original.trampoline( ).address( ) )( REGISTERS_OUT, hdr, pos, quaternion, matrix, mask, computed );
    entity->is_jiggle_bones_enabled( ) = backup_is_jiggle_bones_enabled;
    studio_hdr->flags = backup_bone_flags;
}

void hooks::build_transformations::init( ) {
    original = safetyhook::create_inline( signature::find( "client.dll", XOR( "55 8B EC 83 E4 F0 81 EC ? ? ? ? 56 57 8B F9 8B 0D ? ? ? ? 89 7C 24 1C" ) ).get< void * >( ),
                                          build_transformations::hook );
}