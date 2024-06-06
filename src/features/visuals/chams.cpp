#include "chams.hpp"
#include <features/animations/anims.hpp>
#include <hooks/draw_model_execute/draw_model_execute.hpp>
#include <iterator>

void chams::init( ) {
    auto textured_kv = new key_values( "VertexLitGeneric" );
    {
        textured_kv->set_string( "$basetexture", "vgui/white_additive" );
        textured_kv->set_string( "$flat", "1" );
        textured_kv->set_string( "$alpha", "1.0" );
        textured_kv->set_string( "$ambientonly", "1" );
    }

    textured = g_interfaces.material_system->create_material( "material_textured", textured_kv );
    textured->increment_reference_count( );

    auto flat_kv = new key_values( "UnlitGeneric" );
    {
        flat_kv->set_string( "$basetexture", "vgui/white_additive" );
        flat_kv->set_string( "$flat", "1" );
        flat_kv->set_string( "$alpha", "1.0" );
    }

    flat = g_interfaces.material_system->create_material( "material_flat", flat_kv );
    flat->increment_reference_count( );
}

void chams::draw_model( i_material *material, c_base_entity *entity, color col, bool ignore_z ) {
    //g_interfaces.model_render->forced_material_override( material );
    //g_interfaces.render_view->set_color_modulation( col );
    //g_interfaces.render_view->set_blend( col.a( ) / 255.0f );

    //material->set_material_var_flag( material_var_flags::ignore_z, ignore_z );
    //entity->draw_model( 1 | 128 );
    //forced_material = true;

    //g_interfaces.model_render->forced_material_override( nullptr );
    //g_interfaces.render_view->set_color_modulation( { 255, 255, 255 } );
    //g_interfaces.render_view->set_blend( 1.0f );
}

bool chams::draw_model_execute( i_model_render *model_render, void *edx, void *render_context, const draw_model_state_t &state, const model_render_info_t &info, matrix_3x4 *bone_to_world, c_base_entity *entity ) {
    if ( !g_vars.visuals_render_player_chams_toggle.value )
        return false;

    if ( !globals::local_player )
        return false;

    auto model_name = g_interfaces.model_info->get_model_name( info.model );

    if ( strstr( model_name, XOR( "models/player" ) ) ) {
        auto select_material = [ & ]( ) -> i_material * {
            switch ( g_vars.visuals_render_player_chams_material.value ) {
                case 1: return textured; break;
                case 0: return flat; break;
            }
        };

        selected = select_material( );
        forced_material = false;

        if ( !entity || entity == globals::local_player || entity->team( ) == globals::local_player->team( ) || !entity->is_player( ) || !entity->alive( ) )
            return false;

        using namespace hooks::draw_model_execute;

        auto original = reinterpret_cast< decltype( &hook ) >( hooks::draw_model_execute::original.trampoline( ).address( ) );

        if ( model_render->is_forced_material_override( ) ) 
            return false;

        std::array< matrix_3x4, 128 > out_bones;

        if ( g_vars.visuals_render_player_chams_lag_record.value && g_animations.get_lagcomp_bones( entity->get< c_cs_player * >( ), out_bones ) ) {
            model_render->forced_material_override( selected );
            g_interfaces.render_view->set_color_modulation( g_vars.visuals_render_player_chams_lag_record_color.value );
            g_interfaces.render_view->set_blend( g_vars.visuals_render_player_chams_lag_record_color.value.a / 255.0f );

            selected->set_material_var_flag( material_var_flags::ignore_z, true );
            original( model_render, edx, render_context, state, info, out_bones.data( ) );
            selected->set_material_var_flag( material_var_flags::ignore_z, false );

            model_render->forced_material_override( nullptr );
            g_interfaces.render_view->set_color_modulation( { 255, 255, 255 } );
            g_interfaces.render_view->set_blend( 1.0f );
        }

        if ( g_vars.visuals_render_player_chams_xqz.value ) {
            g_interfaces.model_render->forced_material_override( selected );
            g_interfaces.render_view->set_color_modulation( g_vars.visuals_render_player_chams_xqz_color.value );
            g_interfaces.render_view->set_blend( g_vars.visuals_render_player_chams_xqz_color.value.a / 255.0f );

            selected->set_material_var_flag( material_var_flags::ignore_z, true );
            original( model_render, edx, render_context, state, info, bone_to_world );

            g_interfaces.model_render->forced_material_override( nullptr );
            g_interfaces.render_view->set_color_modulation( { 255, 255, 255 } );
            g_interfaces.render_view->set_blend( 1.0f );
        }

        if ( g_vars.visuals_render_player_chams.value ) {
            g_interfaces.model_render->forced_material_override( selected );
            g_interfaces.render_view->set_color_modulation( g_vars.visuals_render_player_chams_color.value );
            g_interfaces.render_view->set_blend( g_vars.visuals_render_player_chams_color.value.a / 255.0f );

            selected->set_material_var_flag( material_var_flags::ignore_z, false );
            original( model_render, edx, render_context, state, info, bone_to_world );

            g_interfaces.model_render->forced_material_override( nullptr );
            g_interfaces.render_view->set_color_modulation( { 255, 255, 255 } );
            g_interfaces.render_view->set_blend( 1.0f );
        }
    }

    return true;
}

void chams::post_screen_space_effects( c_base_entity *entity ) {
    //auto network_name = HASH( entity->get_client_class( )->network_name );

    //switch ( network_name ) {
    //    case HASH_CT( "CTFPlayer" ): {
    //        const auto player = entity->get< c_tf_player * >( );

    //        if ( !player || player == globals::local_player || player->team( ) == globals::local_player->team( ) )
    //            break;

    //        if ( g_vars.visuals_render_player_chams_xqz.value )
    //            draw_model( selected, entity, ( g_vars.visuals_render_player_chams_show_steam_friend.value && entity->is_player_on_steam_friends( ) ) ? g_vars.visuals_render_player_chams_steam_friend_color.value : g_vars.visuals_render_player_chams_xqz_color.value, true );

    //        if ( g_vars.visuals_render_player_chams.value )
    //            draw_model( selected, entity, ( g_vars.visuals_render_player_chams_show_steam_friend.value && entity->is_player_on_steam_friends( ) ) ? g_vars.visuals_render_player_chams_steam_friend_color.value : g_vars.visuals_render_player_chams_color.value, false );
    //    } break;
    //    case HASH_CT( "CObjectTeleporter" ):
    //    case HASH_CT( "CObjectSentrygun" ):
    //    case HASH_CT( "CObjectDispenser" ): {
    //        const auto building = entity->get< c_base_object * >( );

    //        if ( globals::local_player ) {
    //            if ( building->team( ) == globals::local_player->team( ) )
    //                break;
    //        }

    //        if ( g_vars.visuals_render_building_chams_xqz.value )
    //            draw_model( selected, entity, g_vars.visuals_buildings_render_chams_xqz_color.value, true );

    //        if ( g_vars.visuals_render_building_chams.value )
    //            draw_model( selected, entity, g_vars.visuals_buildings_render_chams_color.value, false );
    //    } break;
    //    default: break;
    //}
}