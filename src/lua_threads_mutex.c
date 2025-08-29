#include "lua_threads_mutex.h"

pthread_mutex_t thread_lock;
pthread_mutex_t pc_mutex;

static int thread_mutex_init(lua_State *L) {
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);
		pthread_mutex_init(&t->mutex, NULL);
		lua_pushlightuserdata(L, &t->mutex);

		return 1;
	}
	lua_pushlightuserdata(L, &thread_lock);

	return 1;
};

static int thread_mutex_destroy(lua_State *L) {
	int ret = -1;
	if(lua_islightuserdata(L, 1)) {
		pthread_mutex_t *mutex = (pthread_mutex_t*)lua_topointer(L, 1);
		ret = pthread_mutex_destroy(mutex);
	}
	lua_pushinteger(L, ret);

	return 1;
};

static int thread_mutex_lock(lua_State *L) {
	if(lua_islightuserdata(L, 1)) {
		pthread_mutex_t *mutex = (pthread_mutex_t*)lua_topointer(L, 1);
		pthread_mutex_lock(mutex);
	}
	return 0;
};

static int thread_mutex_unlock(lua_State *L) {
	if(lua_islightuserdata(L, 1)) {
		pthread_mutex_t *mutex = (pthread_mutex_t*)lua_topointer(L, 1);
		pthread_mutex_unlock(mutex);
	}
	return 0;
};

static const struct luaL_reg thread_mutex_lib_m [] = {
	{"init", 	thread_mutex_init	},
	{"lock", 	thread_mutex_lock	},
	{"unlock",	thread_mutex_unlock	},
	{"destroy",	thread_mutex_destroy	},
	{NULL, NULL}
};

void lua_threads_open_thread_mutex(lua_State *L) {
	lua_getfield(L, -1, "thread");
	lua_newtable(L);
	luaL_register(L, NULL, thread_mutex_lib_m);
	lua_setfield(L, -2, MUTEX);
	// thread_mutex
	luaL_newmetatable(L, "LuaBook.thread_mutex");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);  /* pushes the metatable */
	lua_settable(L, -3);  /* metatable.__index = metatable */
	luaL_openlib(L, NULL, thread_mutex_lib_m, 0);
	lua_pop(L, 1);
};