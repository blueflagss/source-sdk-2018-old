#include "renderer.hpp"
#include <resources/small_font_recreate.hpp>
#include <resources/montserrat.hpp>
#include <resources/montserrat_semibold.hpp>
#include <resources/font_awesome.hpp>
#include <icons_fa.hpp>
#include <globals.hpp>
#include <imgui/imgui_internal.h>

void render::init( HWND hwnd, IDirect3DDevice9 *device ) {
    ImGui::CreateContext( );
    ImGui_ImplWin32_Init( hwnd );
    ImGui_ImplDX9_Init( device );

    auto &io = ImGui::GetIO( );

    fonts::visuals_indicators = create_from_system( "C:\\Windows\\Fonts\\verdanab.ttf", 27.5f, 0, io.Fonts->GetGlyphRangesCyrillic( ) );
    fonts::visuals_04b03 = create_from_system( small_font_data, 8.0f, sizeof( small_font_data ), ImGuiFreeTypeBuilderFlags_LightHinting, io.Fonts->GetGlyphRangesCyrillic( ) );
    fonts::visuals_segoe_ui = create_from_system( "C:\\Windows\\Fonts\\seguisb.ttf", 16.0f, ImGuiFreeTypeBuilderFlags_LightHinting, io.Fonts->GetGlyphRangesCyrillic( ) );
    fonts::montserrat_semibold = create_from_system( montserrat_semibold_data, 16.0f, sizeof( montserrat_semibold_data ), 0, io.Fonts->GetGlyphRangesCyrillic( ) );
    fonts::montserrat = create_from_system( montserrat_data, 15.5f, sizeof( montserrat_data ), 0, nullptr );

    ImFontConfig cfg;

    cfg.MergeMode = true;
    cfg.GlyphMinAdvanceX = 12.0f;

    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

    fonts::montserrat_main = create_from_system( montserrat_data, 13.0f, sizeof( montserrat_data ), 0, io.Fonts->GetGlyphRangesCyrillic( ), &cfg );
    fonts::font_awesome = create_from_system( font_awesome_data, 14.0f, sizeof( font_awesome_data ), 0, icons_ranges, &cfg );

    io.Fonts->Build( );
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
}

void render::begin_draw_states( IDirect3DDevice9 *device ) {
    D3DVIEWPORT9 d3d_viewport;

    device->GetViewport( &d3d_viewport );
    device->CreateStateBlock( D3DSBT_ALL, &pixel_state );
    pixel_state->Capture( );

    device->GetRenderState( D3DRS_COLORWRITEENABLE, &old_D3DRS_COLORWRITEENABLE );
    device->GetRenderState( D3DRS_SRGBWRITEENABLE, &srgbwrite );

    device->SetVertexShader( nullptr );
    device->SetPixelShader( nullptr );
    device->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );

    device->SetRenderState( D3DRS_LIGHTING, FALSE );
    device->SetRenderState( D3DRS_FOGENABLE, FALSE );
    device->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
    device->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
    device->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
    device->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
    device->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
    device->SetRenderState( D3DRS_STENCILENABLE, FALSE );
    device->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, TRUE );
    device->SetRenderState( D3DRS_ANTIALIASEDLINEENABLE, TRUE );
    device->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    device->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
    device->SetRenderState( D3DRS_SEPARATEALPHABLENDENABLE, TRUE );
    device->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
    device->SetRenderState( D3DRS_SRCBLENDALPHA, D3DBLEND_INVDESTALPHA );
    device->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
    device->SetRenderState( D3DRS_DESTBLENDALPHA, D3DBLEND_ONE );
    device->SetRenderState( D3DRS_SRGBWRITEENABLE, FALSE );
    device->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA );
}

void render::finish_draw_states( IDirect3DDevice9 *device ) {
    pixel_state->Apply( );
    pixel_state->Release( );

    device->SetRenderState( D3DRS_COLORWRITEENABLE, old_D3DRS_COLORWRITEENABLE );
    device->SetRenderState( D3DRS_SRGBWRITEENABLE, srgbwrite );
}

bool render::world_to_screen( const glm::vec3 &point, glm::vec2 &screen ) {
    static auto view_matrix = signature::find( "client.dll", "0F 10 05 ? ? ? ? 8D 85 ? ? ? ? B9" ).add( 3 ).deref( ).add( 176 ).get< std::uintptr_t >( );
    const auto &world_matrix = *reinterpret_cast< matrix_3x4 * >( view_matrix );

    screen.x = world_matrix[ 0 ][ 0 ] * point.x + world_matrix[ 0 ][ 1 ] * point.y + world_matrix[ 0 ][ 2 ] * point.z + world_matrix[ 0 ][ 3 ];
    screen.y = world_matrix[ 1 ][ 0 ] * point.x + world_matrix[ 1 ][ 1 ] * point.y + world_matrix[ 1 ][ 2 ] * point.z + world_matrix[ 1 ][ 3 ];

    float width = world_matrix[ 3 ][ 0 ] * point.x + world_matrix[ 3 ][ 1 ] * point.y + world_matrix[ 3 ][ 2 ] * point.z + world_matrix[ 3 ][ 3 ];

    if ( width < 0.001f ) {
        screen.x *= 100000.f;
        screen.y *= 100000.f;
        return false;
    }

    screen.x *= 1.f / width;
    screen.y *= 1.f / width;

    screen.x = ( globals::ui::screen_size.x / 2.f ) + ( screen.x * globals::ui::screen_size.x ) / 2.f;
    screen.y = ( globals::ui::screen_size.y / 2.f ) - ( screen.y * globals::ui::screen_size.y ) / 2.f;

    return true;
}


glm::vec2 render::get_text_size( ImFont *font, const std::string &text ) {
    ImGui::PushFont( font );
    ImVec2 size = ImGui::CalcTextSize( text.c_str( ) );
    ImGui::PopFont( );

    return { size.x, size.y };
}

void render::string( ImFont *font, float x, float y, color col, const std::string &text, bool shadow, bool outline ) {
    ImGui::PushFont( font );

    if ( shadow ) {
        draw_list->AddText( { ImFloor( x + 1 ), ImFloor( y + 1 ) }, IM_COL32( 0, 0, 0, col.a ), text.c_str( ) );
    }

    if ( outline ) {
        draw_list->AddText( { ImFloor( x + 1 ), ImFloor( y ) }, IM_COL32( 0, 0, 0, col.a ), text.c_str( ) );
        draw_list->AddText( { ImFloor( x - 1 ), ImFloor( y ) }, IM_COL32( 0, 0, 0, col.a ), text.c_str( ) );
        draw_list->AddText( { ImFloor( x ), ImFloor( y + 1 ) }, IM_COL32( 0, 0, 0, col.a ), text.c_str( ) );
        draw_list->AddText( { ImFloor( x ), ImFloor( y - 1 ) }, IM_COL32( 0, 0, 0, col.a ), text.c_str( ) );
    }

    draw_list->AddText( { ImFloor( x ), ImFloor( y ) }, IM_COL32( col.r, col.g, col.b, col.a ), text.c_str( ) );

    ImGui::PopFont( );
}

void render::circle_filled( int x, int y, int radius, int segments, color col ) {
    draw_list->AddCircleFilled( { ImFloor( x ), ImFloor( y ) }, radius, IM_COL32( col.r, col.g, col.b, col.a ), segments );
}

void render::circle_filled( glm::vec2 pos, int radius, int segments, color col ) {
    circle_filled( pos.x, pos.y, radius, segments, col );
}

void render::circle( int x, int y, int radius, int segments, color col, float thickness ) {
    draw_list->AddCircle( { ImFloor( x ), ImFloor( y ) }, radius, IM_COL32( col.r, col.g, col.b, col.a ), segments, thickness );
}

void render::circle( glm::vec2 pos, int radius, int segments, color col, float thickness ) {
    circle( pos.x, pos.y, radius, segments, col, thickness );
}

void render::string( ImFont *font, glm::vec2 pos, color col, const std::string &text, bool shadow, bool outline ) {
    string( font, pos.x, pos.y, col, text, shadow, outline );
}

ImFont *render::create_from_system( const std::string &font_path, float font_size, const int flags,
                                    const ImWchar *glyph_ranges ) {
    ImFontConfig cfg = { };
    cfg.FontBuilderFlags |= flags;
    return ImGui::GetIO( ).Fonts->AddFontFromFileTTF( font_path.c_str( ), font_size, &cfg, glyph_ranges );
}

ImFont *render::create_from_system( std::uint8_t *data, float font_size, size_t data_size, const int flags,
                                    const ImWchar *glyph_ranges ) {
    ImFontConfig cfg = { };
    cfg.FontBuilderFlags |= flags;
    return ImGui::GetIO( ).Fonts->AddFontFromMemoryTTF( data, data_size, font_size, &cfg, glyph_ranges );
}

ImFont *render::create_from_system( std::uint8_t *data, float font_size, size_t data_size, const int flags, const ImWchar *glyph_ranges, ImFontConfig *cfg ) {
    cfg->FontBuilderFlags |= flags;
    return ImGui::GetIO( ).Fonts->AddFontFromMemoryTTF( data, data_size, font_size, cfg, glyph_ranges );
}

void render::filled_rect( int x, int y, int w, int h, color col, float rounding, int corner_flags ) {
    draw_list->AddRectFilled( { ImFloor( x ), ImFloor( y ) }, { ImFloor( x + w ), ImFloor( y + h ) },
                              IM_COL32( col.r, col.g, col.b, col.a ), rounding, corner_flags );
}

void render::filled_rect( glm::vec2 pos, glm::vec2 size, color col, float rounding, int corner_flags ) {
    filled_rect( pos.x, pos.y, size.x, size.y, col, rounding, corner_flags );
}

void render::triangle( int x1, int y1, int x2, int y2, int x3, int y3, color col, float thickness ) {
    draw_list->AddTriangle( { ImFloor( x1 ), ImFloor( y1 ) }, { ImFloor( x2 ), ImFloor( y2 ) }, { ImFloor( x3 ), ImFloor( y3 ) }, IM_COL32( col.r, col.g, col.b, col.a ), thickness );
}

void render::triangle( std::array< glm::vec2, 3 > points, color col, float thickness ) {
    triangle( points[ 0 ].x, points[ 0 ].y, points[ 1 ].x, points[ 1 ].y, points[ 2 ].x, points[ 2 ].y, col, thickness );
}

void render::filled_triangle( int x1, int y1, int x2, int y2, int x3, int y3, color col ) {
    draw_list->AddTriangleFilled( { ImFloor( x1 ), ImFloor( y1 ) }, { ImFloor( x2 ), ImFloor( y2 ) }, { ImFloor( x3 ), ImFloor( y3 ) }, IM_COL32( col.r, col.g, col.b, col.a ) );
}

void render::filled_triangle( std::array< glm::vec2, 3 > points, color col ) {
    filled_triangle( points[ 0 ].x, points[ 0 ].y, points[ 1 ].x, points[ 1 ].y, points[ 2 ].x, points[ 2 ].y, col );
}

void render::gradient_rect( int x, int y, int w, int h, color c1, color c2, bool vertical ) {
    draw_list->AddRectFilledMultiColor(
            { ImFloor( x ), ImFloor( y ) },
            { ImFloor( x + w ), ImFloor( y + h ) },
            IM_COL32( c1.r, c1.g, c1.b, c1.a ),
            vertical ? ImColor{ c1.r, c1.g, c1.b, c1.a } : ImColor{ c2.r, c2.g, c2.b, c2.a },
            IM_COL32( c2.r, c2.g, c2.b, c2.a ),
            vertical ? ImColor{ c2.r, c2.g, c2.b, c2.a } : ImColor{ c1.r, c1.g, c1.b, c1.a } );
}

void render::gradient_rect( glm::vec2 pos, glm::vec2 size, color c1, color c2, bool vertical ) {
    gradient_rect( pos.x, pos.y, size.x, size.y, c1, c2, vertical );
}

void render::line( int x, int y, int x0, int y0, color col, float thickness ) {
    draw_list->AddLine( { ImFloor( x ), ImFloor( y ) }, { ImFloor( x0 ), ImFloor( y0 ) },
                        IM_COL32( col.r, col.g, col.b, col.a ), thickness );
}

void render::line( glm::vec2 point_a, glm::vec2 point_b, color col, float thickness ) {
    line( point_a.x, point_a.y, point_b.x, point_b.y, col, thickness );
}

void render::multi_gradient_rect( int x, int y, int w, int h, color c1, color c2, color c3, color c4 ) {
    draw_list->AddRectFilledMultiColor(
            { ImFloor( x ), ImFloor( y ) },
            { ImFloor( x + w ), ImFloor( y + h ) },
            IM_COL32( c1.r, c1.g, c1.b, c1.a ),
            IM_COL32( c2.r, c2.g, c2.b, c2.a ),
            IM_COL32( c3.r, c3.g, c3.b, c3.a ),
            IM_COL32( c4.r, c4.g, c4.b, c4.a ) );
}

void render::multi_gradient_rect( glm::vec2 pos, glm::vec2 size, color c1, color c2, color c3, color c4 ) {
    multi_gradient_rect( pos.x, pos.y, size.x, size.y, c1, c2, c3, c4 );
}

void render::rect( int x, int y, int w, int h, color col, float rounding, int corner_flags ) {
    draw_list->AddRect( { ImFloor( x ), ImFloor( y ) }, { ImFloor( x + w ), ImFloor( y + h ) },
                        IM_COL32( col.r, col.g, col.b, col.a ), rounding, corner_flags, 1.f );
}

void render::scissor_rect( float x, float y, float w, float h, const std::function< void( ) > &func ) {
    draw_list->PushClipRect( { ImFloor( x ), ImFloor( y ) }, { ImFloor( x + w ), ImFloor( y + h ) }, true );

    func( );

    draw_list->PopClipRect( );
}

void render::scissor_rect( glm::vec2 pos, glm::vec2 size, const std::function< void( ) > &func ) {
    scissor_rect( pos.x, pos.y, size.x, size.y, func );
}

void render::rect( glm::vec2 pos, glm::vec2 size, color col, float rounding, int corner_flags ) {
    rect( pos.x, pos.y, size.x, size.y, col, rounding, corner_flags );
}

void render::show_objects( ) {
    ImGui::Render( );
    ImGui_ImplDX9_RenderDrawData( ImGui::GetDrawData( ) );
}

void render::start( ) {
    ImGui_ImplDX9_NewFrame( );
    ImGui_ImplWin32_NewFrame( );
    ImGui::NewFrame( );

    // Set draw list.
    draw_list = ImGui::GetBackgroundDrawList( );
    // draw_list->Flags &= ~( ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines );
}

void render::begin( const std::function< void( ) > &fn ) {
    start( );
    {
        fn( );
    }
    finish( );
}

void render::finish( ) {
    ImGui::EndFrame( );
}