#pragma once

class i_client_entity;
class i_client_networkable;
class i_client_unknown;

class c_entity_list {
public:
    virtual i_client_networkable *get_client_networkable( int index ) = 0;
    virtual i_client_networkable *get_client_networkable_from_handle( uint32_t entity_handle ) = 0;
    virtual i_client_unknown *get_client_unknown_from_handle( uint32_t entity_handle ) = 0;
    virtual i_client_entity *get_client_entity( int index ) = 0;
    virtual i_client_entity *get_client_entity_from_handle( uint32_t entity_handle ) = 0;
    virtual int number_of_entities( bool include_non_networkable ) = 0;
    virtual int get_highest_entity_index( void ) = 0;
    virtual void set_max_entities( int max_clients ) = 0;
    virtual int get_max_entities( ) = 0;

    template< typename T >
    __inline T get_client_entity( int index ) {
        return reinterpret_cast< T >( this->get_client_entity( index ) );
    }

    template< typename T >
    __inline T get_client_entity_from_handle( uint32_t entity_handle ) {
        return reinterpret_cast< T >( this->get_client_entity_from_handle( entity_handle ) );
    }
};
