#pragma once

class i_client_unknown;
class client_think_handle;

class i_client_thinkable {
public:
    virtual i_client_unknown *get_client_unknown( ) = 0;
    virtual void client_think( ) = 0;
    virtual client_think_handle get_think_handle( ) = 0;
    virtual void set_think_handle( client_think_handle think ) = 0;
    virtual void release( ) = 0;
};