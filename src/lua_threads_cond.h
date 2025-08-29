#ifndef THREADS_COND_H
#define THREADS_COND_H

#define COND "cond"
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif

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
#include <time.h>
#include <string.h>

#include <pthread.h>

#ifdef __cplusplus
}
#endif
void lua_threads_open_thread_cond(lua_State *L);

#endif
