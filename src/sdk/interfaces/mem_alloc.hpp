#pragma once

class c_mem_alloc {
public:
    void *alloc( size_t sz ) {
        return utils::get_method< void *( __thiscall * ) ( void *, size_t ) >( this, 1 )( this, sz );
    }

    void *realloc( void *mem, size_t sz ) {
        return utils::get_method< void *( __thiscall * ) ( void *, void *, size_t ) >( this, 3 )( this, mem, sz );
    }

    void free( void *mem ) {
        return utils::get_method< void( __thiscall * )( void *, void * ) >( this, 5 )( this, mem );
    }
};