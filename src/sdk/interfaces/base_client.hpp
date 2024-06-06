#pragma once
#include <utils/utils.hpp>

class c_client_class;
class c_recv_table;
class c_recv_prop;
class c_recv_proxy_data;
class c_variant;
class c_view_setup;

enum should_transmit_state : int {
    should_transmit_start = 0,
    should_transmit_end
};

enum data_update_type : int {
    data_update_created = 0,
    data_update_datatable_changed,
};

enum send_prop_type : int {
    dpt_int = 0,
    dpt_float,
    dpt_vector,
    dpt_vectorxy,
    dpt_string,
    dpt_array,
    dpt_datatable,
    dpt_int64,
    dpt_numsendproptypes
};

using create_client_class_fn = void *( * ) ( int, int );
using create_event_fn = void *( * ) ( );

using recv_var_proxy_fn = void ( * )( const c_recv_proxy_data *, void *, void * );
using array_length_recv_proxy_fn = void ( * )( void *, int, int );
using data_table_recv_var_proxy_fn = void ( * )( const c_recv_prop *, void **, void *, int );

enum client_frame_stage : int {
    frame_undefined = -1,
    frame_start,
    frame_net_update_start,
    frame_net_update_postdataupdate_start,
    frame_net_update_postdataupdate_end,
    frame_net_update_end,
    frame_render_start,
    frame_render_end
};

class c_variant {
public:
    union {
        float f;
        long i;
        char *s;
        void *data;
        float v[ 3 ];
        long long i64;
    };

    send_prop_type type;
};

class c_recv_proxy_data {
public:
    const c_recv_prop *recv_prop;
    c_variant value;
    int element;
    int object_id;
};

class c_recv_prop {
public:
    char *var_name;
    send_prop_type recv_type;
    int flags;
    int string_buffer_size;
    bool inside_array;
    const void *extra_data;
    c_recv_prop *array_prop;
    array_length_recv_proxy_fn array_length_proxy;
    recv_var_proxy_fn proxy_fn;
    data_table_recv_var_proxy_fn data_table_proxy_fn;
    c_recv_table *data_table;
    int offset;
    int element_stride;
    int num_elements;
    const char *parent_array_prop_name;

    __inline const char* get_type_name( ) {
        switch ( recv_type ) {
            case dpt_int:
                return "int";
            case dpt_float:
                return "float";
            case dpt_vector:
                return "vector_3d";
            case dpt_vectorxy:
                return "vector_2d";
            case dpt_array:
                return "array";
            case dpt_string:
                return "const char*";
            case dpt_datatable:
                return "void*";
            case dpt_int64:
                return "__int64";
            case dpt_numsendproptypes:
                return "unk";
        }
    }
};

class c_client_class {
public:
    create_client_class_fn create_fn;
    create_event_fn create_event_fn;
    char *network_name;
    c_recv_table *recv_table;
    c_client_class *next;
    int class_id;
    const char *map_class_name;
};

class c_recv_table {
public:
    c_recv_prop *props;
    int num_props;
    void *decoder;
    char *net_table_name;
    bool initialized;
    bool in_main_list;
};

class c_base_client {
public:
    __inline c_client_class *get_all_classes( ) {
        return utils::get_method< c_client_class *( __thiscall * ) ( void * ) >( this, 8 )( this );
    }

    __inline bool get_player_view( c_view_setup &setup ) {
        return utils::get_method< bool( __thiscall * )( void *, c_view_setup & ) >( this, 59 )( this, setup );
    }
};