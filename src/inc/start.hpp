

#pragma once

#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>

void register_callback(struct evhttp *http);