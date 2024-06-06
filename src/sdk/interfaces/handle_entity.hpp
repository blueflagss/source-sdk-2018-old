#pragma once

class i_handle_entity {
public:
    virtual ~i_handle_entity( ) {}
    virtual void set_ref_handle( const uint16_t &handle ) = 0;
    virtual const uint16_t &get_ref_handle( ) const = 0;
};