#include "lua_threads_sem.h"

sem_t thread_sem_lock;

static int thread_sem_init(lua_State *L) {
	uint8_t pshared = luaL_optinteger(L, 1, 1);
	uint8_t locks = luaL_optinteger(L, 2, 1);
	sem_init(&thread_sem_lock, pshared, locks); // currently limited to 1 sem ...needs rethinking 
	lua_pushlightuserdata(L, &thread_sem_lock);

	return 1;
};

static int thread_sem_destroy(lua_State *L) {
	int ret = 1;
	if(lua_islightuserdata(L, 1)) {
		sem_t *thread_sem_lock = (sem_t*)lua_topointer(L, 1);
		ret = sem_destroy(thread_sem_lock);
	}
	lua_pushinteger(L, ret);

	return 1;
};

static int thread_sem_wait(lua_State *L) {
	int ret = 1;
	if(lua_islightuserdata(L, 1)) {
		sem_t *thread_sem_lock = (sem_t*)lua_topointer(L, 1);
		ret = sem_wait(thread_sem_lock);
	}
	lua_pushinteger(L, ret);

	return 1;
};

static int thread_sem_trywait(lua_State *L) {
	int ret = 1;
	if(lua_islightuserdata(L, 1)) {
		sem_t *thread_sem_lock = (sem_t*)lua_topointer(L, 1);
		ret = sem_trywait(thread_sem_lock);
	}
	lua_pushinteger(L, ret);

	return 1;
};

/*
static int thread_sem_timedwait(lua_State *L) {
	int ret = -1;
	if(lua_islightuserdata(L, 1)) {
		sem_t *thread_sem_lock = (sem_t*)lua_topointer(L, 1);
		time_t sec = (time_t)luaL_optinteger(L, 2, 0);
		long nsec = luaL_optlong(L, 3, 0);

		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += sec;
		ts.tv_nsec += nsec;

		ret = sem_timedwait(thread_sem_lock, &ts);
	}
	lua_pushinteger(L, ret);

	return 1;
};
*/

static int thread_sem_post(lua_State *L) {
	int ret = 1;
	if(lua_islightuserdata(L, 1)) {
		sem_t *thread_sem_lock = (sem_t*)lua_topointer(L, 1);
		ret = sem_post(thread_sem_lock);
	}
	lua_pushinteger(L, ret);

	return 1;
};

static const struct luaL_reg thread_sem_lib_m [] = {
	{"init", 	thread_sem_init		},
	{"wait", 	thread_sem_wait		},
	{"trywait", 	thread_sem_trywait	},
	{"post",	thread_sem_post		},
	{"destroy",	thread_sem_destroy	},
	{NULL, NULL}
};

void lua_threads_open_thread_sem(lua_State *L) {
	lua_newtable(L);
	luaL_register(L, NULL, thread_sem_lib_m);
	lua_setfield(L, -2, SEM);
	// thread_sem
	luaL_newmetatable(L, "LuaBook.thread_sem");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);  /* pushes the metatable */
	lua_settable(L, -3);  /* metatable.__index = metatable */
	luaL_openlib(L, NULL, thread_sem_lib_m, 0);
	lua_pop(L, 1);
};