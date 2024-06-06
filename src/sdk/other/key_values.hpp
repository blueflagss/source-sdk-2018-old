#pragma once
#include <globals.hpp>

class key_values {
public:
    key_values( const char *setName );

    class auto_delete {
    public:
        explicit inline auto_delete( key_values *key_values ) : key_value( key_values ) { }
        explicit inline auto_delete( const char *kv_name ) : key_value( new key_values( kv_name ) ) { }

        inline void assign( key_values *key_value ) { key_value = key_value; }
        key_values *operator->( ) { return key_value; }
        operator key_values *( ) { return key_value; }

    private:
        auto_delete( auto_delete const &x );
        auto_delete &operator=( auto_delete const &x );
        key_values *key_value;
    };

    key_values *find_key( const char *key_name, bool create );
    void set_string( const char *key_name, const char *value );
    void set_int( const char *key_name, int value );

    key_values *get_first_subkey( ) { return sub; }
    key_values *get_next_key( ) { return peer; }

    void set_bool( const char *key_name, bool value ) { set_int( key_name, value ? 1 : 0 ); }

    void *operator new( size_t size ) {
        return malloc( size );
    }

    void operator delete( void *ptr ) {
        free( ptr );
    }

    enum types {
        TYPE_NONE = 0,
        TYPE_STRING,
        TYPE_INT,
        TYPE_FLOAT,
        TYPE_PTR,
        TYPE_WSTRING,
        TYPE_COLOR,
        TYPE_UINT64,
        TYPE_NUMTYPES,
    };

    int key_name;
    char *_string;
    wchar_t *_wstring;

    union {
        int _int;
        float _float;
        void *_ptr;
        unsigned char _color[ 4 ];
    };

    char data_type;
    char has_escape_sequences;
    char evalulate_conditionals;
    char unused[ 1 ];

    key_values *peer;
    key_values *sub;
    key_values *chain;
};