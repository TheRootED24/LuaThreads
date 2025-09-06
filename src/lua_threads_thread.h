#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#define THREAD "thread"

#define MAX_THREADS 150 
#define PTHREAD_STACK_MIN  16384
#define STACK_SIZE  PTHREAD_STACK_MIN * 10

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
#include <sched.h> 
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdatomic.h>

#include "lua_threads_mutex.h"
#include "lua_threads_sem.h"
#include "lua_threads_attr.h"
#include "lua_threads_cond.h"
#include "lua_threads_thread_buf.h"
#include "lua_threads_thread_queue.h"

#ifdef __cplusplus
}
#endif

typedef enum {
	WORKING,
	SUSPENDED,
	COMPLETE,
	ERROR,
	OK,
	START,
} state_t;

typedef struct {
	pthread_t thread;
	pthread_mutex_t mutex;
	sem_t joinable;
	pthread_attr_t attr;
	pthread_cond_t wait_cond;
	pthread_condattr_t cond_attr;
	size_t tid;
	sem_t sem;

	_Atomic int id;
	_Atomic state_t state;
	_Atomic bool run;
	_Atomic bool done;

	lua_State *TS;
	lua_State *T;
	const char *fn;
	void *fn_data;
	void *stack_addr;
	size_t stack_size;
	//luaL_Buffer *b;

} threads_thread_t;

void lua_threads_open_threads_thread (lua_State *L);
threads_thread_t *check_threads_thread(lua_State *L, int pos);

#endif

