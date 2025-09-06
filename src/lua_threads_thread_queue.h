#ifndef THREADS_QUEUE_H
#define THREADS_QUEUE_H

#define QUEUE "queue"

// C++ GAURDS FOR C LIBS
#ifndef __cplusplus
// LUA LIBS FOR gcc
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#endif

#ifdef __cplusplus
// LUA LIBS FOR g++
#include <lua.hpp>
extern "C" {
#endif

// STANDARD C LIBS
#include <stdint.h>
#include <pthread.h>
#include "lua_threads_thread.h"

#ifdef __cplusplus
}
#endif

void lua_threads_open_thread_queue(lua_State *L);

#endif
