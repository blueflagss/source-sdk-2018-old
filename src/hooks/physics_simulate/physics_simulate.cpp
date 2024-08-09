#include "physics_simulate.hpp"
#include <features/prediction_manager/prediction_manager.hpp>

void __fastcall hooks::physics_simulate::hook( REGISTERS ) {
    auto player = reinterpret_cast< c_cs_player * >( ecx );

    if ( !player || ( ( *reinterpret_cast< int * >( reinterpret_cast< uintptr_t >( player ) + 0x2A8 ) == g_interfaces.global_vars->tick_count ) || !*reinterpret_cast< bool * >( reinterpret_cast< uintptr_t >( player ) + 0x34D0 ) ) )
        return original.fastcall< void >( REGISTERS_OUT );

    if ( player == globals::local_player )
        *reinterpret_cast< int * >( reinterpret_cast< uintptr_t >( globals::local_player ) + 0x3238 ) = 0;

    if ( player == globals::local_player && globals::local_player->alive( ) ) {
        //auto copy_layers = []( c_animation_layer *to, c_animation_layer *from, bool copy_jump_anims ) -> void {
        //    if ( copy_jump_anims ) {
        //        for ( int i = 0; i < ANIMATION_LAYER_COUNT; i++ ) {
        //            switch ( i ) {
        //                case ANIMATION_LAYER_ALIVELOOP:
        //                case ANIMATION_LAYER_LEAN:
        //                case ANIMATION_LAYER_WHOLE_BODY:
        //                case ANIMATION_LAYER_MOVEMENT_MOVE:
        //                case ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL:
        //                case ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB: {
        //                    to[ i ].weight_delta_rate = from[ i ].weight_delta_rate;
        //                    to[ i ].sequence = from[ i ].sequence;
        //                    to[ i ].previous_cycle = from[ i ].previous_cycle;
        //                    to[ i ].weight = from[ i ].weight;
        //                    to[ i ].playback_rate = from[ i ].playback_rate;
        //                    to[ i ].cycle = from[ i ].cycle;
        //                    to[ i ].owner = from[ i ].owner;
        //                } break;
        //            }
        //        }
        //    } else {
        //        for ( int i = 0; i < ANIMATION_LAYER_COUNT; i++ ) {
        //            switch ( i ) {
        //                case ANIMATION_LAYER_ALIVELOOP:
        //                case ANIMATION_LAYER_WHOLE_BODY:
        //                case ANIMATION_LAYER_LEAN:
        //                case ANIMATION_LAYER_MOVEMENT_MOVE:
        //                case ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL:
        //                case ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB: {
        //                    to[ i ].weight_delta_rate = from[ i ].weight_delta_rate;
        //                    to[ i ].sequence = from[ i ].sequence;
        //                    to[ i ].previous_cycle = from[ i ].previous_cycle;
        //                    to[ i ].weight = from[ i ].weight;
        //                    to[ i ].playback_rate = from[ i ].playback_rate;

        //                    to[ i ].cycle = from[ i ].cycle;

        //                    to[ i ].owner = from[ i ].owner;
        //                } break;
        //            }
        //        }
        //    }
        //};
    }

    original.fastcall< void >( REGISTERS_OUT );

    if ( player == globals::local_player && player->viewmodel_handle( ) != 0xFFFFFFF ) {
        const auto viewmodel = g_interfaces.entity_list->get_client_entity_from_handle< c_view_model * >( player->viewmodel_handle( ) );

        if ( viewmodel ) {
            g_prediction_context.weapon_cycle = viewmodel->cycle( );
            g_prediction_context.weapon_sequence = viewmodel->sequence( );
        }
    }
}

void hooks::physics_simulate::init( ) {
    original = safetyhook::create_inline( signature::find( _xs( "client.dll" ), _xs( "56 8B F1 8B 8E ? ? ? ? 83 F9 FF 74 21" ) ).get< void * >( ),
                                          physics_simulate::hook );
}