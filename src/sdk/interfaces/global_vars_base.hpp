#pragma once

class c_global_vars_base {
public:
    float realtime;             // 0x0000
    int framecount;             // 0x0004
    float abs_frametime;        // 0x0008
    float abs_framestarttime;   // 0x000C
    float curtime;              // 0x0010
    float frametime;            // 0x0014
    int max_clients;            // 0x0018
    int tick_count;             // 0x001C
    float interval_per_tick;    // 0x0020
    float interpolation_amount; // 0x0024
    int simticks_this_frame;    // 0x0028
    int network_protocol;       // 0x002C

    // only call from your instance of globals vars base
    void backup( c_global_vars_base* from ) {
        this->realtime = from->realtime;
        this->framecount = from->framecount;
        this->abs_frametime = from->abs_frametime;
        this->curtime = from->curtime;
        this->frametime = from->frametime;
        this->max_clients = from->max_clients;
        this->tick_count = from->tick_count;
        this->interval_per_tick = from->interval_per_tick;
        this->interpolation_amount = from->interpolation_amount;
        this->simticks_this_frame = from->simticks_this_frame;
        this->network_protocol = from->network_protocol;
    }

    // only call from interface global vars base
    void restore( const c_global_vars_base &to ) {
        this->realtime = to.realtime;
        this->framecount = to.framecount;
        this->abs_frametime = to.abs_frametime;
        this->curtime = to.curtime;
        this->frametime = to.frametime;
        this->max_clients = to.max_clients;
        this->tick_count = to.tick_count;
        this->interval_per_tick = to.interval_per_tick;
        this->interpolation_amount = to.interpolation_amount;
        this->network_protocol = to.network_protocol;
    }
};