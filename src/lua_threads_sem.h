#ifndef THREADS_SEM_H
#define THREADS_SEM_H

#define SEM "sem"
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
#include <semaphore.h>


#ifdef __cplusplus
}
#endif
void lua_threads_open_thread_sem(lua_State *L);

#endif
