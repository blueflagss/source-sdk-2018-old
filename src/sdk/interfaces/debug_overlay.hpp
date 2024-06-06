#pragma once

class overlaytext_t {
public:
    overlaytext_t( ) {
        next_overlay_text = 0;
        use_origin = false;
        line_offset = 0;
        x_pos = 0;
        y_pos = 0;
        text[ 0 ] = 0;
        end_time = 0.0f;
        server_count = -1;
        creation_tick = -1;
        r = g = b = a = 255;
    }

    vector_3d origin;
    bool use_origin;
    int line_offset;
    float x_pos;
    float y_pos;
    char text[ 512 ];
    float end_time;   // when does this text go away
    int creation_tick;// if > 0, show only one server frame
    int server_count; // compare server spawn count to remove stale overlays
    int r;
    int g;
    int b;
    int a;
    overlaytext_t *next_overlay_text;
};

struct i_debug_overlay {
public:
    virtual void add_entity_text_overlay( int ent_index, int line_offset, float duration, int r, int g, int b, int a, const char *format, ... ) = 0;
    virtual void add_box_overlay( const vector_3d &origin, const vector_3d &mins, const vector_3d &max, vector_3d const &orientation, int r, int g, int b, int a, float duration ) = 0;
    virtual void add_sphere_overlay( const vector_3d &origin, float radius, int theta, int phi, int r, int g, int b, int a, float duration ) = 0;
    virtual void add_triangle_overlay( const vector_3d &p1, const vector_3d &p2, const vector_3d &p3, int r, int g, int b, int a, bool nodepthtest, float duration ) = 0;
    virtual void pad001( ) = 0;
    virtual void pad002( ) = 0;
    virtual void add_text_overlay( const vector_3d &origin, float duration, const char *format, ... ) = 0;
    virtual void add_text_overlay( const vector_3d &origin, int line_offset, float duration, const char *format, ... ) = 0;
    virtual void add_screen_text_overlay( float flxpos, float flypos, float flduration, int r, int g, int b, int a, const char *text ) = 0;
    virtual void add_swept_box_overlay( const vector_3d &start, const vector_3d &end, const vector_3d &mins, const vector_3d &max, const vector_3d &angles, int r, int g, int b, int a, float flduration ) = 0;
    virtual void add_grid_overlay( const vector_3d &origin ) = 0;
    virtual int screen_position( const vector_3d &point, vector_3d &screen ) = 0;
    virtual int screen_position( float flxpos, float flypos, vector_3d &screen ) = 0;
    virtual overlaytext_t *get_first( void ) = 0;
    virtual overlaytext_t *get_next( overlaytext_t *current ) = 0;
    virtual void clear_dead_overlays( void ) = 0;
    virtual void clear_all_overlays( ) = 0;
    virtual void add_text_overlay_rgb( const vector_3d &origin, int line_offset, float duration, float r, float g, float b, float alpha, const char *format, ... ) = 0;
    virtual void add_text_overlay_rgb( const vector_3d &origin, int line_offset, float duration, int r, int g, int b, int a, const char *format, ... ) = 0;
    virtual void add_line_overlay_alpha( const vector_3d &origin, const vector_3d &dest, int r, int g, int b, int a, bool nodepthtest, float duration ) = 0;
    virtual void add_box_overlay2( const vector_3d &origin, const vector_3d &mins, const vector_3d &max, vector_3d const &orientation, const color &facecolor, const color &edgecolor, float duration ) = 0;

    void add_line_overlay( const vector_3d& origin, const vector_3d& dest, int r, int g, int b, bool nodepthtest, float duration ) {
        return utils::get_method< void( __thiscall * )( void *, const vector_3d &, const vector_3d &, int, int, int, bool, float ) >( this, 5 )( this, origin, dest, r, g, b, nodepthtest, duration );
    }
};