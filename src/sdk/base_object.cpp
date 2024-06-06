#include "base_object.hpp"

std::string c_base_object::object_name( ) {
    switch ( HASH( this->get_client_class( )->network_name ) ) {
        case HASH_CT( "CObjectDispenser" ):  return "Dispenser";
        case HASH_CT( "CObjectTeleporter" ): return "Teleporter";
        case HASH_CT( "CObjectSentrygun" ):  return "Sentry";
        default: break;
    }

    return { };
}