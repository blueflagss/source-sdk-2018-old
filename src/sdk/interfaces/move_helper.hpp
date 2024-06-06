#pragma once
#include <utils/utils.hpp>

class c_base_player;

class c_move_helper {
public:
    virtual void pad00( ) = 0;
    virtual void set_host( void *host ) = 0;
    virtual void pad01( ) = 0;
    virtual void pad02( ) = 0;
    virtual void process_impacts( ) = 0;
};