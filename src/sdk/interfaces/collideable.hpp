#pragma once

class i_handle_entity;

class i_collideable {
public:
    virtual i_handle_entity *get_entity_handle( ) = 0;
    virtual const vector_3d &mins( ) const = 0;
    virtual const vector_3d &maxs( ) const = 0;
};