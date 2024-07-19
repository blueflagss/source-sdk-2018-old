#include "post_network_data_received.hpp"
#include <sdk/other/prediction_copy.hpp>
#include <features/network_data/network_data.hpp>
#include <features/animations/animation_sync.hpp>

void __fastcall hooks::post_network_data_received::hook( REGISTERS, int commands_acknowledged ) {
    globals::local_player = g_interfaces.entity_list->get_client_entity< c_cs_player * >( g_interfaces.engine_client->get_local_player( ) );

    static auto build_flattened_chains = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 EC 18 57 8B F9 89 7D F4 83 7F 14 00 0F 85 ? ? ? ?" ) ).get< void( __thiscall * )( datamap_t * ) >( );

    if ( globals::local_player ) {
        const auto map = globals::local_player->get_pred_desc_map( );

        if ( map ) {
            if ( !globals::did_setup_datamap ) {
                // adding vars to the pred datamap
                //const typedescription_t type_description{
                //        FIELD_FLOAT,
                //        "m_flVelocityModifier",
                //        g_netvars.get_offset( HASH_CT( "DT_CSPlayer" ), HASH_CT( "m_flVelocityModifier" ) ),
                //        1,
                //        0x100,
                //        "",
                //        sizeof( float ),
                //         };

                static auto velocity_modifier_offset = g_netvars.get_offset( HASH_CT( "DT_CSPlayer" ), HASH_CT( "m_flVelocityModifier" ) );

                typedescription_t type_description;

                type_description.field_type = FIELD_FLOAT;
                type_description.field_name = _xs( "m_flVelocityModifier" );
                type_description.field_offset = velocity_modifier_offset;
                type_description.field_size = 1;
                type_description.flags = 0x100;
                type_description.external_name = "";
                type_description.field_size_in_bytes = sizeof( float );
                type_description.field_tolerance = assign_range_multiplier( 8, 1.0f ); 
                const auto type_array = new typedescription_t[ map->data_num_fields + 1 ];

                memcpy( type_array, map->data_desc, sizeof( typedescription_t ) * map->data_num_fields );

                type_array[ map->data_num_fields ] = type_description;

                map->optimized_datamap = nullptr;
                map->data_desc = type_array;
                map->data_num_fields++;
                map->packed_size = 0;

                build_flattened_chains( map );

                // init pred vars
                g_network_data.init( globals::local_player );

                globals::did_setup_datamap = true;
            }
        }

        if ( globals::local_player->alive( ) && commands_acknowledged > 0 ) {
            //memcpy( g_animations.tranny_code_premium_layers.data( ), globals::local_player->anim_overlays( ), g_animations.tranny_code_premium_layers.size( ) );
            g_network_data.post_update( globals::local_player );
        }
    }

    return original.fastcall< void >( REGISTERS_OUT, commands_acknowledged );
}

void hooks::post_network_data_received::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.prediction, 6 ),
                                          post_network_data_received::hook );
}