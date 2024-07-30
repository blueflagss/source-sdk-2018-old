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