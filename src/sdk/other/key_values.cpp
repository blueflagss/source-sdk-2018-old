#include "key_values.hpp"

key_values::key_values( const char *set_name ) {
    static auto key_values = signature::find( "engine.dll", "E8 ? ? ? ? 8B F8 6A 01" ).add( 0x1 ).rel32( ).get< void( __thiscall * )( void *, const char * ) >( );

    key_values( this, set_name );
}

key_values *key_values::find_key( const char *key_name, bool create ) {
    static auto find_key = signature::find( "engine.dll", "E8 ? ? ? ? 5B 85 C0" ).add( 0x1 ).rel32( ).get< key_values *( __thiscall * ) ( void *, const char *, bool ) >( );

    return find_key( this, key_name, create );
}

void key_values::set_string( const char *key_name, const char *value ) {
    key_values *dat = find_key( key_name, true );

    if ( dat ) {
        if ( dat->data_type == TYPE_STRING && dat->_string == value ) {
            return;
        }

        delete[ ] dat->_string;
        delete[ ] dat->_wstring;
        dat->_wstring = NULL;

        if ( !value ) {
            value = "";
        }

        int len = strlen( value );
        dat->_string = new char[ len + 1 ];
        memcpy( dat->_string, value, len + 1 );

        dat->data_type = TYPE_STRING;
    }
}

void key_values::set_int( const char *key_name, int value ) {
    key_values *dat = find_key( key_name, true );

    if ( dat ) {
        dat->_int = value;
        dat->data_type = TYPE_INT;
    }
}