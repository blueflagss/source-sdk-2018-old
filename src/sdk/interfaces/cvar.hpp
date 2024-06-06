#pragma once
#include <globals.hpp>
#include <sdk/hash/fnv1a.hpp>

class convar;
class command;
class con_command;
class c_con_command_base;
class i_con_command_base_accessor;

enum cvar_flags_t : uint32_t {
    FVAR_UNREGISTERED = ( 1 << 0 ),
    FVAR_DEVELOPMENTONLY = ( 1 << 1 ),
    FVAR_GAMEDLL = ( 1 << 2 ),
    FVAR_CLIENTDLL = ( 1 << 3 ),
    FVAR_HIDDEN = ( 1 << 4 ),
    FVAR_PROTECTED_VAR = ( 1 << 5 ),
    FVAR_SPONLY = ( 1 << 6 ),
    FVAR_ARCHIVE = ( 1 << 7 ),
    FVAR_NOTIFY = ( 1 << 8 ),
    FVAR_USERINFO = ( 1 << 9 ),
    FVAR_PRINTABLEONLY = ( 1 << 10 ),
    FVAR_UNLOGGED = ( 1 << 11 ),
    FVAR_NEVER_AS_STRING = ( 1 << 12 ),
};

class convar;

typedef void ( *change_callback_t )( convar *var, const char *old_value, float fl_old_value );

struct cvar_value_t {
    char *str;
    int str_length;
    float float_value;
    int int_value;
};

class convar {
private:
    void *vtable;

public:
    convar *next;
    int registered;
    char *name;
    char *help_string;
    int flags;
    void *callback;
    convar *parent;
    char *default_value;
    cvar_value_t value;
    int has_min;
    float min;
    int has_max;
    float max;
    void *callbacks;

public:
    void set_int( int value ) {
        return utils::get_method< void( __thiscall * )( void *, int ) >( this, 16 )( this, value );
    }

    void set_float( float value ) {
        return utils::get_method< void( __thiscall * )( void *, float ) >( this, 15 )( this, value );
    }

    void set_char( const char *value ) {
        return utils::get_method< void( __thiscall * )( void *, const char * ) >( this, 14 )( this, value );
    }

    void remove_callbacks( ) {
        *reinterpret_cast< int * >( reinterpret_cast< uintptr_t >( &callbacks ) + 0xC ) = 0;
    }

    const char *get_name( ) {
        return utils::get_method< const char *( __thiscall * ) ( void * ) >( this, 5 )( this );
    }

    const char *get_string( ) {
        if ( flags & FVAR_NEVER_AS_STRING )
            return "FCVAR_NEVER_AS_STRING";

        return value.str ? value.str : "";
    }

    bool get_bool( ) {
        return !!get_int( );
    }

    float get_float( ) {
        uint32_t xored = *reinterpret_cast< uintptr_t * >( &value.float_value ) ^ reinterpret_cast< uintptr_t >( this );
        return *reinterpret_cast< float * >( &xored );
    }

    int get_int( ) {
        return static_cast< int >( value.int_value ^ reinterpret_cast< uintptr_t >( this ) );
    }
};

class i_app_system {
public:
    virtual bool connect( void *factory ) = 0;
    virtual void disconnect( ) = 0;
    virtual void *query_interface( const char *interface_name ) = 0;
    virtual int init( ) = 0;
    virtual void shutdown( char *reason ) = 0;
};

class i_cvar {
public:
    void console_color_printf( const color &col, const char *format, ... ) {
        return utils::get_method< void( __cdecl * )( void *, const color &, const char *, ... ) >( this, 25 )( this, col, format );
    }

    void console_printf( const char *format, ... ) {
        return utils::get_method< void( __cdecl * )( void *, const char *, ... ) >( this, 26 )( this, format );
    }

    convar *get_commands( ) {
        return *reinterpret_cast< convar ** >( reinterpret_cast< uintptr_t >( this ) + 0x30 );
    }

    convar *find_var( const hash32_t &hash ) {
        convar *cvar_list;

        cvar_list = get_commands( );

        if ( !cvar_list )
            return nullptr;

        for ( auto it = cvar_list; it != nullptr; it = it->next ) {
            if ( HASH( it->name ) == hash ) {
                return it;
            }
        }

        return nullptr;
    }
};