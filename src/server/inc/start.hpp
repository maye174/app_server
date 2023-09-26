

#pragma once

#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>

#include <tuple>
#include <vector>

typedef struct {
    int id;
    struct event *ev;
    struct timeval interval;
} timer_data;

std::vector<timer_data *> register_timer(struct event_base *base);
void register_callback(struct event_base *base, struct evhttp *http);