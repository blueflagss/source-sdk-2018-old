#pragma once

enum net_msg : int {
    nop = 0,
    disconnect,
    file,
    tick,
    string_cmd,
    set_convar,
    signon_state,
    client_info = 8,
    move,
    voice_data,
    baseline_ack,
    listen_events,
    respond_cvar_value,
    file_crc_check,
    cmd_key_values
};