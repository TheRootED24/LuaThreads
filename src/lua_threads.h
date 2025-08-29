#ifndef LUA_THREADS_H
#define LUA_THREADS_H

#define THREADS "threads"

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
// EXTRA C LIBS
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

#include "lua_threads_sem.h"
#include "lua_threads_mutex.h"
#include "lua_threads_thread.h"

#ifdef __cplusplus
}
#endif
int threads_errs(lua_State *L);

#endif