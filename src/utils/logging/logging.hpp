#pragma once
#include <globals.hpp>

class logging {
public:
    void init( );
    void remove( );
};

inline logging g_logging = { };