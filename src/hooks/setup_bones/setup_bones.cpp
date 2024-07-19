#include "setup_bones.hpp"
#include <features/animations/animation_sync.hpp>

bool __fastcall hooks::setup_bones::hook( REGISTERS, matrix_3x4 *out, int bones, int mask, float curtime ) {
    auto base_entity = reinterpret_cast< c_cs_player * >( reinterpret_cast< i_client_renderable * >( ecx )->get_client_unknown( )->get_base_entity( ) );

    if ( !base_entity || !base_entity->alive( ) )
        return original.fastcall< bool >( REGISTERS_OUT, out, out, mask, curtime );

    static auto &g_model_bone_counter = *signature::find( _xs( "client.dll" ), _xs( "3B 05 ? ? ? ? 0F 84 ? ? ? ? 8B 47" ) ).add( 2 ).deref( ).get< int * >( );
 
    auto owner = base_entity->get_root_move_parent( );
    auto main_entity = owner ? owner : base_entity;

    if ( main_entity->is_player( ) && !globals::is_building_bones[ base_entity->index( ) ] ) {
        static auto attachment_helper = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 EC 48 53 8B 5D 08 89 4D F4 56 57 85 DB 0F 84" ) ).get< void( __thiscall * )( void *, void * ) >( );

        auto base_animating = reinterpret_cast< c_base_entity * >( ecx );

        if ( *reinterpret_cast< int * >( reinterpret_cast< uintptr_t >( base_animating ) + 0x2680 ) != g_model_bone_counter ) {
            memcpy( base_animating->bone_cache( ), g_animations.animated_bones[ main_entity->index( ) ].data( ), sizeof( matrix_3x4 ) * base_animating->bone_count( ) );

            /* apply animated bone origin. */
            for ( auto i = 0; i < base_animating->bone_count( ); i++ )
                base_animating->bone_cache( )[ i ].set_origin( base_animating->bone_cache( )[ i ].get_origin( ) - g_animations.animated_origin[ main_entity->index( ) ] + base_entity->get_render_origin( ) );

            if ( base_entity->cstudio_hdr( ) )
                attachment_helper( main_entity, base_entity->cstudio_hdr( ) );

            *reinterpret_cast< int * >( reinterpret_cast< uintptr_t >( base_animating ) + 0x2680 ) = g_model_bone_counter;
        }

        if ( out ) {
            if ( bones >= base_animating->bone_count( ) )
                memcpy( out, base_animating->bone_cache( ), sizeof( matrix_3x4 ) * base_animating->bone_count( ) );
            else
                return false;
        }

        return true;
    }

    return original.fastcall< bool >( REGISTERS_OUT, out, bones, mask, curtime );
}

void hooks::setup_bones::init( ) {
    original = safetyhook::create_inline( signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 57 8B F9 8B 0D" ) ).get< void * >( ),
                                          setup_bones::hook );
}