#pragma once

class i_handle_entity {
public:
    virtual ~i_handle_entity( ) {}
    virtual void set_ref_handle( const uint32_t &handle ) = 0;
    virtual const uint32_t &get_ref_handle( ) const = 0;
};