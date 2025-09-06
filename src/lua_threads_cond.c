#include "lua_threads_cond.h"

static int thread_cond_new(lua_State *L) {
	pthread_cond_t *cond = (pthread_cond_t*)lua_newuserdata(L, sizeof(pthread_cond_t));
	memset(cond, 0, sizeof(pthread_cond_t));
	pthread_cond_init(cond, NULL);

	return 1;
}

static int thread_cond_destroy(lua_State *L) {
	int ret = -1;
	if(lua_isuserdata(L, 1)) {
		pthread_cond_t *cond = (pthread_cond_t *)lua_topointer(L, 1);
		pthread_cond_destroy(cond);
	}
	lua_pushinteger(L, ret);

	return 1;
}

static int thread_cond_wait(lua_State *L) {
	int ret = -1;
	if(lua_isuserdata(L, 1) && lua_isuserdata(L, 2)) {
		pthread_cond_t *cond = (pthread_cond_t *)lua_topointer(L, 1);
		pthread_mutex_t *mutex = (pthread_mutex_t*)lua_topointer(L, 2);
		ret = pthread_cond_wait(cond, mutex);
	}
	lua_pushinteger(L, ret);

	return 1;
}

static int thread_cond_timedwait(lua_State *L) {
	int ret = -1;
	if(lua_isuserdata(L, 1) && lua_isuserdata(L, 2)) {
		pthread_cond_t *cond = (pthread_cond_t *)lua_topointer(L, 1);
		pthread_mutex_t *mutex = (pthread_mutex_t*)lua_topointer(L, 2);
		time_t sec = 0;
		long nsec = 0;
		if(lua_istable(L, 3)) {
			lua_rawgeti(L, 3, 1);
			sec = (time_t)luaL_optinteger(L, -1, 0);
			lua_rawgeti(L, 3, 2);
			nsec = (time_t)luaL_optinteger(L, -1, 0);
			lua_pop(L, 2);
		}
		else {
			sec = (time_t)luaL_optinteger(L, 3, 0);
			nsec = luaL_optlong(L, 4, 0);
		}

		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += sec;
		ts.tv_nsec += nsec;

		ret = pthread_cond_timedwait(cond, mutex, &ts);
	}
	lua_pushinteger(L, ret);

	return 1;
};

static int thread_cond_broadcast(lua_State *L) {
	int ret = -1;
	if(lua_isuserdata(L, 1)) {
		pthread_cond_t *cond = (pthread_cond_t *)lua_topointer(L, 1);
		ret = pthread_cond_broadcast(cond);
	}
	lua_pushinteger(L, ret);

	return 1;
}

static int thread_cond_signal(lua_State *L) {
	int ret = -1;
	if(lua_isuserdata(L, 1)) {
		pthread_cond_t *cond = (pthread_cond_t *)lua_topointer(L, 1);
		ret = pthread_cond_signal(cond); // 0 for producer
	}
	lua_pushinteger(L, ret);

	return 1;
}

static const struct luaL_reg thread_cond_lib_f[] = {
	{ "new",	thread_cond_new		},
	{ "wait",	thread_cond_wait	},
	{ "timedwait",	thread_cond_timedwait	},
	{ "broadcast", 	thread_cond_broadcast	},
	{ "signal",	thread_cond_signal	},
	{ "destroy",	thread_cond_destroy	},
	{NULL, NULL}
};

static const struct luaL_reg thread_cond_lib_m[] = {
	{ "new",	thread_cond_new		},
	{ "wait",	thread_cond_wait	},
	{ "timedwait",	thread_cond_timedwait	},
	{ "broadcast", 	thread_cond_broadcast	},
	{ "signal",	thread_cond_signal	},
	{ "destroy",	thread_cond_destroy	},
	{NULL, NULL}
};

void lua_threads_open_thread_cond(lua_State *L) {
	lua_newtable(L);
	luaL_register(L, NULL, thread_cond_lib_m);
	lua_setfield(L, -2, COND);
	// thread_cond
	luaL_newmetatable(L, "LuaBook.thread_cond");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);  /* pushes the metatable */
	lua_settable(L, -3);  /* metatable.__index = metatable */
	luaL_openlib(L, NULL, thread_cond_lib_m, 0);
	luaL_openlib(L, "threads_cond", thread_cond_lib_f, 0);
	
	lua_pop(L, 2);
};