#pragma once

class i_client_networkable;
class i_client_renderable;
class i_client_thinkable;
class i_handle_entity;
class i_collideable;

class i_client_unknown : public i_handle_entity {
public:
    virtual i_collideable *collideable( ) = 0;
    virtual i_client_networkable *get_client_networkable( ) = 0;
    virtual i_client_renderable *get_client_renderable( ) = 0;
    virtual void *get_client_entity( ) = 0;
    virtual void *get_base_entity( ) = 0;
    virtual i_client_thinkable *get_client_thinkable( ) = 0;
    virtual void *get_client_alpha_property( ) = 0;
};