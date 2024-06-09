#include "utlvector.hpp"
#include <globals.hpp>

void *utils::alloc( size_t sz ) {
    return g_interfaces.mem_alloc->alloc( sz );
}

void *utils::realloc( void *mem, size_t sz ) {
    return g_interfaces.mem_alloc->realloc( mem, sz );
}

void utils::free_memory( void *mem ) {
    g_interfaces.mem_alloc->free( mem );
}
