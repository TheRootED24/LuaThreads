
// beta version
#include "lua_threads.h"

extern int thread_errs;
extern int active_threads;
extern int yielded_threads;
extern int completed_threads;

static int threads_active(lua_State *L) {
		lua_pushinteger(L, active_threads);

	return 1;
};

static int threads_yielded(lua_State *L) {
		lua_pushinteger(L, yielded_threads);

	return 1;
};

static int threads_completed(lua_State *L) {
		lua_pushinteger(L, completed_threads);

	return 1;
};

int threads_errored(lua_State *L) {
		lua_pushinteger(L, thread_errs);
	return 1;
};

static int threads_stats(lua_State *L) {
	if(L)
		printf("\nActive Threads: %d\nYielded Threads: %d\nCompleted Threads: %d\n\n", active_threads, yielded_threads, completed_threads);

	return 0;
};

static const luaL_reg threads_methods[] = {
	{ "stats",	threads_stats		},
	{ "completed",	threads_completed	},
	{ "yielded",	threads_yielded		},
	{ "active",	threads_active		},
	{ "errored",	threads_errored		},
	{NULL, NULL}
};

int luaopen_threads(lua_State *L) {
	// threads
	luaL_newmetatable(L, "threads.threads");
	lua_pushvalue(L, -1);	// pushes the metatable
	lua_setfield(L, -2, "__index");	// metatable.__index = metatable
	luaL_register(L, THREADS, threads_methods);
	// thread state enum constants
	lua_pushinteger(L, WORKING);
	lua_setglobal(L, "WORKING");
	lua_pushinteger(L, SUSPENDED);
	lua_setglobal(L, "SUSPENDED");
	lua_pushinteger(L, COMPLETE);
	lua_setglobal(L, "COMPLETE");
	lua_pushinteger(L, ERROR);
	lua_setglobal(L, "ERROR");
	lua_pushinteger(L, OK);
	lua_setglobal(L, "OK");

	//open thread module
	lua_threads_open_threads_thread (L);

	return 1;
};
