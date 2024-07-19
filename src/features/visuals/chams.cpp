#include "chams.hpp"
#include <features/animations/animation_sync.hpp>
#include <features/resolver/resolver.hpp>
#include <hooks/draw_model_execute/draw_model_execute.hpp>
#include <iterator>

void chams::init( ) {
    auto textured_kv = new key_values( _xs( "VertexLitGeneric" ) );
    {
        textured_kv->set_string( _xs( "$basetexture" ), _xs( "vgui/white_additive" ) );
        textured_kv->set_string( _xs( "$flat" ), _xs( "1" ) );
        textured_kv->set_string( _xs( "$alpha" ), _xs( "1.0" ) );
        textured_kv->set_string( _xs( "$ambientonly" ), _xs( "1" ) );
    }

    textured = g_interfaces.material_system->create_material( _xs( "material_textured" ), textured_kv );
    textured->increment_reference_count( );

    auto flat_kv = new key_values( _xs( "UnlitGeneric" ) );
    {
        flat_kv->set_string( _xs( "$basetexture" ), _xs( "vgui/white_additive" ) );
        flat_kv->set_string( _xs( "$flat" ), _xs( "1" ) );
        flat_kv->set_string( _xs( "$alpha" ), _xs( "1.0" ) );
    }

    flat = g_interfaces.material_system->create_material( _xs( "material_flat" ), flat_kv );
    flat->increment_reference_count( );
}

void chams::draw_model( i_material *material, c_base_entity *entity, color col, bool ignore_z ) {
    g_interfaces.model_render->forced_material_override( material );
    g_interfaces.render_view->set_color_modulation( col );
    g_interfaces.render_view->set_blend( col.a / 255.0f );

    material->set_material_var_flag( material_var_flags::ignore_z, ignore_z );
    entity->draw_model( 1 | 128 );
    forced_material = true;

    g_interfaces.model_render->forced_material_override( nullptr );
    g_interfaces.render_view->set_color_modulation( { 255, 255, 255 } );
    g_interfaces.render_view->set_blend( 1.0f );
}

bool chams::draw_model_execute( i_model_render *model_render, void *edx, void *render_context, const draw_model_state_t &state, const model_render_info_t &info, matrix_3x4 *bone_to_world, c_base_entity *entity ) {
    if ( !g_vars.visuals_render_player_chams_toggle.value )
        return false;

    if ( !globals::local_player )
        return false;

    const auto model_name = g_interfaces.model_info->get_model_name( info.model );

    if ( strstr( model_name, _xs( "models/player" ) ) ) {
        forced_material = false;

        auto select_material = [ & ]( ) -> i_material * {
            switch ( g_vars.visuals_render_player_chams_material.value ) {
                case 1:
                    return textured;
                    break;
                case 0:
                    return flat;
                    break;
            }
        };

        selected = select_material( );
        forced_material = false;

        if ( !entity )
            return false;

        using namespace hooks::draw_model_execute;

        auto original = reinterpret_cast< decltype( &hook ) >( hooks::draw_model_execute::original.trampoline( ).address( ) );

        if ( model_render->is_forced_material_override( ) )
            return false;

        if ( entity == globals::local_player && globals::local_player->alive( ) ) {
            if ( g_vars.visuals_other_local_blend_while_scoped.value && globals::local_player->scoped( ) )
                g_interfaces.render_view->set_blend( g_vars.visuals_other_local_blend_transparency.value / 100.0f );
        }

        if ( entity == globals::local_player || entity->team( ) == globals::local_player->team( ) )
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

void chams::on_post_screen_space_effects( ) {
    if ( !g_vars.visual_players_toggled.value )
        return;

    if ( !globals::local_player )
        return;

    for ( int i = 0; i < g_interfaces.glow_object_manager->glow_object_definitions.m_Size; i++ ) {
        auto &glow_object = g_interfaces.glow_object_manager->glow_object_definitions.Element( i );

        if ( glow_object.is_unused( ) || !glow_object.entity )
            continue;

        auto player = reinterpret_cast< c_cs_player * >( glow_object.entity );

        if ( !player->alive( ) || player->team( ) == globals::local_player->team( ) || !player->is_player( ) )
            continue;

        glow_object.set( static_cast< float >( g_vars.visuals_render_player_glow_color.value.r ) / 255.0f, static_cast< float >( g_vars.visuals_render_player_glow_color.value.g ) / 255.0f, static_cast< float >( g_vars.visuals_render_player_glow_color.value.b ) / 255.0f, static_cast< float >( g_vars.visuals_render_player_glow_color.value.a ) / 255.0f );
    }
}