#include "skin_changer.hpp"

class ccstrike15itemschema;
class ccstrike15itemsystem;

template< typename key, typename value >
struct node_t {
    int previous_id;
    int next_id;
    void *_unknown_ptr;
    int _unknown;
    key key;
    value value;
};

template< typename key, typename value >
struct head_t {
    node_t< key, value > *memory;
    int allocation_count;
    int grow_size;
    int start_element;
    int next_available;
    int _unknown;
    int last_element;
};

struct string_t {
    char *buffer;
    int capacity;
    int grow_size;
    int length;
};

struct c_paintkit {
    int id;

    string_t name;
    string_t description;
    string_t item_name;
    string_t material_name;
    string_t image_inventory;

    char pad_0x0054[ 0x8c ];
};

std::array< int, 20 > knife_types = {
        skin_changer::WEAPON_KNIFE,
        skin_changer::WEAPON_KNIFE_BAYONET,
        skin_changer::WEAPON_KNIFE_CSS,
        skin_changer::WEAPON_KNIFE_FLIP,
        skin_changer::WEAPON_KNIFE_GUT,
        skin_changer::WEAPON_KNIFE_KARAMBIT,
        skin_changer::WEAPON_KNIFE_M9_BAYONET,
        skin_changer::WEAPON_KNIFE_TACTICAL,
        skin_changer::WEAPON_KNIFE_FALCHION,
        skin_changer::WEAPON_KNIFE_SURVIVAL_BOWIE,
        skin_changer::WEAPON_KNIFE_BUTTERFLY,
        skin_changer::WEAPON_KNIFE_PUSH,
        skin_changer::WEAPON_KNIFE_CORD,
        skin_changer::WEAPON_KNIFE_CANIS,
        skin_changer::WEAPON_KNIFE_URSUS,
        skin_changer::WEAPON_KNIFE_GYPSY_JACKKNIFE,
        skin_changer::WEAPON_KNIFE_OUTDOOR,
        skin_changer::WEAPON_KNIFE_STILETTO,
        skin_changer::WEAPON_KNIFE_WIDOWMAKER,
        skin_changer::WEAPON_KNIFE_SKELETON };

std::array< const char *, 20 > knife_models = {
        "models/weapons/v_knife_default_ct.mdl",
        "models/weapons/v_knife_bayonet.mdl",
        "models/weapons/v_knife_css.mdl",
        "models/weapons/v_knife_flip.mdl",
        "models/weapons/v_knife_gut.mdl",
        "models/weapons/v_knife_karam.mdl",
        "models/weapons/v_knife_m9_bay.mdl",
        "models/weapons/v_knife_tactical.mdl",
        "models/weapons/v_knife_falchion_advanced.mdl",
        "models/weapons/v_knife_survival_bowie.mdl",
        "models/weapons/v_knife_butterfly.mdl",
        "models/weapons/v_knife_push.mdl",
        "models/weapons/v_knife_cord.mdl",
        "models/weapons/v_knife_canis.mdl",
        "models/weapons/v_knife_ursus.mdl",
        "models/weapons/v_knife_gypsy_jackknife.mdl",
        "models/weapons/v_knife_outdoor.mdl",
        "models/weapons/v_knife_stiletto.mdl",
        "models/weapons/v_knife_widowmaker.mdl",
        "models/weapons/v_knife_skeleton.mdl" };

void skin_changer::init( ) {
    const auto V_UCS2ToUTF8 = reinterpret_cast< int ( * )( const wchar_t *ucs2, char *utf8, int len ) >( GetProcAddress( GetModuleHandleA( _xs( "vstdlib.dll" ) ), _xs( "V_UCS2ToUTF8" ) ) );

    static auto item_system_fn = signature::find( _xs( "client.dll" ), _xs( "E8 ? ? ? ? 0F B7 0F" ) ).add( 0x1 ).rel32( ).get< ccstrike15itemsystem *( * ) ( ) >( );

    const auto item_schema = reinterpret_cast< ccstrike15itemschema * >( std::uintptr_t( item_system_fn( ) ) + sizeof( void * ) );
    static auto get_paint_kit_definition_fn = signature::find( _xs( "client.dll" ), _xs( "E8 ? ? ? ? 8B F0 8B 4E 6C" ) ).add( 0x1 ).rel32( ).get< c_paintkit *( __thiscall * ) ( ccstrike15itemschema *, int ) >( );
    const auto start_element_offset = *reinterpret_cast< std::intptr_t * >( std::uintptr_t( get_paint_kit_definition_fn ) + 8 + 2 );
    const auto head_offset = start_element_offset - 12;
    const auto map_head = reinterpret_cast< head_t< int, c_paintkit * > * >( std::uintptr_t( item_schema ) + head_offset );

    for ( auto i = 0; i <= map_head->last_element; ++i ) {
        const auto paint_kit = map_head->memory[ i ].value;

        if ( paint_kit->id == 9001 )
            continue;

        const auto wide_name = g_interfaces.localize->find( paint_kit->item_name.buffer + 1 );
        char name[ 256 ];
        V_UCS2ToUTF8( wide_name, name, sizeof( name ) );

        if ( paint_kit->id < 10000 )
            skin_kits.push_back( { paint_kit->id, name } );
        else
            glove_kits.push_back( { paint_kit->id, name } );
    }

    std::sort( skin_kits.begin( ), skin_kits.end( ) );
    std::sort( glove_kits.begin( ), glove_kits.end( ) );
}

void skin_changer::apply_knifes( ) {
}

void skin_changer::force_full_update( ) {
    if ( !g_interfaces.client_state || !globals::local_player || !globals::local_player->alive( ) )
        return;

    g_interfaces.client_state->delta_tick( ) = -1;
}

int random_sequence( const int low, const int high ) {
    return rand( ) % ( high - low + 1 ) + low;
}

int get_new_animation( const hash32_t &model, const int sequence ) {
    enum ESequence {
        SEQUENCE_DEFAULT_DRAW = 0,
        SEQUENCE_DEFAULT_IDLE1 = 1,
        SEQUENCE_DEFAULT_IDLE2 = 2,
        SEQUENCE_DEFAULT_LIGHT_MISS1 = 3,
        SEQUENCE_DEFAULT_LIGHT_MISS2 = 4,
        SEQUENCE_DEFAULT_HEAVY_MISS1 = 9,
        SEQUENCE_DEFAULT_HEAVY_HIT1 = 10,
        SEQUENCE_DEFAULT_HEAVY_BACKSTAB = 11,
        SEQUENCE_DEFAULT_LOOKAT01 = 12,
        SEQUENCE_BUTTERFLY_DRAW = 0,
        SEQUENCE_BUTTERFLY_DRAW2 = 1,
        SEQUENCE_BUTTERFLY_LOOKAT01 = 13,
        SEQUENCE_BUTTERFLY_LOOKAT03 = 15,
        SEQUENCE_FALCHION_IDLE1 = 1,
        SEQUENCE_FALCHION_HEAVY_MISS1 = 8,
        SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP = 9,
        SEQUENCE_FALCHION_LOOKAT01 = 12,
        SEQUENCE_FALCHION_LOOKAT02 = 13,
        SEQUENCE_CSS_LOOKAT01 = 14,
        SEQUENCE_CSS_LOOKAT02 = 15,
        SEQUENCE_DAGGERS_IDLE1 = 1,
        SEQUENCE_DAGGERS_LIGHT_MISS1 = 2,
        SEQUENCE_DAGGERS_LIGHT_MISS5 = 6,
        SEQUENCE_DAGGERS_HEAVY_MISS2 = 11,
        SEQUENCE_DAGGERS_HEAVY_MISS1 = 12,
        SEQUENCE_BOWIE_IDLE1 = 1,
    };

    switch ( model ) {
        case HASH_CT( "models/weapons/v_knife_butterfly.mdl" ): {
            switch ( sequence ) {
                case SEQUENCE_DEFAULT_DRAW:
                    return random_sequence( SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2 );
                case SEQUENCE_DEFAULT_LOOKAT01:
                    return random_sequence( SEQUENCE_BUTTERFLY_LOOKAT01, SEQUENCE_BUTTERFLY_LOOKAT03 );
                default:
                    return sequence + 1;
            }
        }
        case HASH_CT( "models/weapons/v_knife_falchion_advanced.mdl" ): {
            switch ( sequence ) {
                case SEQUENCE_DEFAULT_IDLE2:
                    return SEQUENCE_FALCHION_IDLE1;
                case SEQUENCE_DEFAULT_HEAVY_MISS1:
                    return random_sequence( SEQUENCE_FALCHION_HEAVY_MISS1, SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP );
                case SEQUENCE_DEFAULT_LOOKAT01:
                    return random_sequence( SEQUENCE_FALCHION_LOOKAT01, SEQUENCE_FALCHION_LOOKAT02 );
                case SEQUENCE_DEFAULT_DRAW:
                case SEQUENCE_DEFAULT_IDLE1:
                    return sequence;
                default:
                    return sequence - 1;
            }
        }
        case HASH_CT( "models/weapons/v_knife_css.mdl" ): {
            switch ( sequence ) {
                case SEQUENCE_DEFAULT_LOOKAT01:
                    return random_sequence( SEQUENCE_CSS_LOOKAT01, SEQUENCE_CSS_LOOKAT02 );
                default:
                    return sequence;
            }
        }
        case HASH_CT( "models/weapons/v_knife_push.mdl" ): {
            switch ( sequence ) {
                case SEQUENCE_DEFAULT_IDLE2:
                    return SEQUENCE_DAGGERS_IDLE1;
                case SEQUENCE_DEFAULT_LIGHT_MISS1:
                case SEQUENCE_DEFAULT_LIGHT_MISS2:
                    return random_sequence( SEQUENCE_DAGGERS_LIGHT_MISS1, SEQUENCE_DAGGERS_LIGHT_MISS5 );
                case SEQUENCE_DEFAULT_HEAVY_MISS1:
                    return random_sequence( SEQUENCE_DAGGERS_HEAVY_MISS2, SEQUENCE_DAGGERS_HEAVY_MISS1 );
                case SEQUENCE_DEFAULT_HEAVY_HIT1:
                case SEQUENCE_DEFAULT_HEAVY_BACKSTAB:
                case SEQUENCE_DEFAULT_LOOKAT01:
                    return sequence + 3;
                case SEQUENCE_DEFAULT_DRAW:
                case SEQUENCE_DEFAULT_IDLE1:
                    return sequence;
                default:
                    return sequence + 2;
            }
        }
        case HASH_CT( "models/weapons/v_knife_survival_bowie.mdl" ): {
            switch ( sequence ) {
                case SEQUENCE_DEFAULT_DRAW:
                case SEQUENCE_DEFAULT_IDLE1:
                    return sequence;
                case SEQUENCE_DEFAULT_IDLE2:
                    return SEQUENCE_BOWIE_IDLE1;
                default:
                    return sequence - 1;
            }
        }
        case HASH_CT( "models/weapons/v_knife_ursus.mdl" ):
        case HASH_CT( "models/weapons/v_knife_cord.mdl" ):
        case HASH_CT( "models/weapons/v_knife_canis.mdl" ):
        case HASH_CT( "models/weapons/v_knife_outdoor.mdl" ):
        case HASH_CT( "models/weapons/v_knife_skeleton.mdl" ): {
            switch ( sequence ) {
                case SEQUENCE_DEFAULT_DRAW:
                    return random_sequence( SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2 );
                case SEQUENCE_DEFAULT_LOOKAT01:
                    return random_sequence( SEQUENCE_BUTTERFLY_LOOKAT01, 14 );
                default:
                    return sequence + 1;
            }
        }
        case HASH_CT( "models/weapons/v_knife_stiletto.mdl" ): {
            switch ( sequence ) {
                case SEQUENCE_DEFAULT_LOOKAT01:
                    return random_sequence( 12, 13 );
                default:
                    return sequence;
            }
        }
        case HASH_CT( "models/weapons/v_knife_widowmaker.mdl" ): {
            switch ( sequence ) {
                case SEQUENCE_DEFAULT_LOOKAT01:
                    return random_sequence( 14, 15 );
                default:
                    return sequence;
            }
        }
        default:
            return sequence;
    }
}

void skin_changer::do_sequence_remapping( c_recv_proxy_data *data, c_base_view_model *entity ) {
    if ( !globals::local_player || !globals::local_player->alive( ) )
        return;

    const auto owner = g_interfaces.entity_list->get_client_entity_from_handle< c_base_player * >( entity->owner_entity( ) );

    if ( owner != globals::local_player )
        return;

    const auto view_model_weapon = g_interfaces.entity_list->get_client_entity_from_handle< c_base_attribute_item * >( entity->weapon( ) );

    if ( !view_model_weapon )
        return;

    const auto weapon_info = knife_types[ view_model_weapon->item_definition_index( ) ];

    if ( !weapon_info )
        return;

    const auto override_model = knife_models[ view_model_weapon->item_definition_index( ) ];

    auto &sequence = data->value.i;
    sequence = get_new_animation( HASH( override_model ), sequence );
}

void skin_changer::on_post_data_update_start( ) {
    if ( !globals::local_player || !globals::local_weapon || !globals::local_player->alive( ) )
        return;

    player_info_t player_info;
    g_interfaces.engine_client->get_player_info( globals::local_player->index( ), &player_info );

    const auto view_model = globals::local_player->get_view_model( );

    if ( !view_model )
        return;

    const auto backup_model_index = view_model->model_index( );

    auto weapons = globals::local_player->weapons( );
    auto current_weapon_definition_index = globals::local_weapon->item_definition_index( );

    static int last_knife_model_index = 0;
    static int last_paintkit_index = 0;

    for ( auto &weapon : weapons ) {
        if ( !weapon )
            continue;

        auto definition_index = weapon->item_definition_index( );

        if ( g_skin_vars[ 0 ][ "knife_override_model" ].b ) {
            auto wanted_definition_index = knife_types[ g_skin_vars[ 0 ][ "knife_model" ].i ];
            auto model_index = g_interfaces.model_info->get_model_index( knife_models[ g_skin_vars[ 0 ][ "knife_model" ].i ] );

            if ( definition_index == WEAPON_KNIFE_T || definition_index == WEAPON_KNIFE || ( definition_index >= WEAPON_KNIFE_BAYONET && definition_index <= WEAPON_KNIFE_SKELETON ) ) {
                weapon->original_owner_xuid_low( ) = player_info.xuid_low;

                const auto backup_definition_index = definition_index;

                weapon->item_definition_index( ) = wanted_definition_index;
                weapon->model_index( ) = model_index;
                weapon->set_model_index( model_index );
                //weapon->view_model_index( ) = model_index;
                //weapon->world_model_index( ) = model_index + 1;
                weapon->entity_quality( ) = 3;

                const auto world_model = g_interfaces.entity_list->get_client_entity_from_handle< c_cs_weapon_base * >( weapon->weapon_world_model( ) );

                if ( world_model )
                    world_model->model_index( ) = model_index + 1;
            }

            if ( weapon->fallback_stattrak( ) >= 0 )
                weapon->entity_quality( ) = 9;

            weapon->fallback_wear( ) = 0.000001f;
            weapon->fallback_stattrak( ) = 1337;
            weapon->account_id( ) = player_info.xuid_low;
            weapon->fallback_paintkit( ) = skin_kits[ g_skin_vars[ current_weapon_definition_index ][ "paintkit" ].i ].id;
            weapon->item_id_high( ) = -1;

            if ( view_model ) {
                if ( current_weapon_definition_index == WEAPON_KNIFE_T || current_weapon_definition_index == WEAPON_KNIFE || ( current_weapon_definition_index >= WEAPON_KNIFE_BAYONET && current_weapon_definition_index <= WEAPON_KNIFE_SKELETON ) ) {
                    view_model->model_index( ) = model_index;
                }
            }

            if ( g_skin_vars[ 0 ][ "knife_override_model" ].b && ( ( last_knife_model_index != g_skin_vars[ 0 ][ "knife_model" ].i ) || ( last_paintkit_index != g_skin_vars[ definition_index ][ "paintkit" ].i ) ) ) {
                last_knife_model_index = g_skin_vars[ 0 ][ "knife_model" ].i;
                last_paintkit_index = g_skin_vars[ definition_index ][ "paintkit" ].i;

                force_full_update( );
            }
        }
    }
}