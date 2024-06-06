#pragma once
#include "globals.hpp"

class c_base_object : public c_base_entity {
public:
    NETVAR( max_health, int, "DT_BaseObject", "m_iMaxHealth" );
    NETVAR( health, int, "DT_BaseObject", "m_iHealth" );
    NETVAR( building, bool, "DT_BaseObject", "m_bBuilding" );
    NETVAR( disabled, bool, "DT_BaseObject", "m_bDisabled" );
    NETVAR( builder, int, "DT_BaseObject", "m_hBuilder" );
    NETVAR( mode, int, "DT_BaseObject", "m_iObjectMode" );

    std::string object_name( );

    c_base_entity* owner( ) {
        if ( !building( ) )
            return nullptr;

        return g_interfaces.entity_list->get_client_entity_from_handle< c_base_entity * >( builder( ) );
    }

    inline bool alive( ) {
        return this->health( ) > 0;
    }
};
