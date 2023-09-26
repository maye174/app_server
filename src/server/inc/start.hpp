

#pragma once

#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>

#include <vector>

std::vector<struct event *> register_timer(struct event_base *base,
                                           struct evhttp *http);
void register_callback(struct event_base *base, struct evhttp *http);