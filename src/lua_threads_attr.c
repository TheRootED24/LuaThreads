#include "lua_threads_attr.h"

// each thread has its own initialized attr
// just add to it as needed before calling create
/*static int thread_attr_init(lua_State *L) {
	threads_thread_t *t = check_threads_thread(L, 1);
	int ret = -1;
	// initialize a default attribute
	ret = pthread_attr_init(&t->attr);
	lua_pushinteger(L, ret);

	return 1;
};
*/

// not really needed .. attr is destroyed with thread
static int thread_attr_destroy(lua_State *L) {
	int ret = -1;
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, 2);

		/* destroy initialized attribute */
		ret = pthread_attr_destroy(&t->attr);
	}

	lua_pushinteger(L, ret);

	return 1;
};

bool show = false;

static int thread_attr_detachstate(lua_State *L) {
	int nargs = lua_gettop(L);
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);
		int ret = -1, detach_state = -1;

		if(nargs > 1) {
			/* set attribute detachedstate */
			ret = pthread_attr_setdetachstate(&t->attr, PTHREAD_CREATE_DETACHED);
			lua_pushinteger(L, ret);

			return 1;
		}

		/* get attribute detachedstate */
		ret = pthread_attr_getdetachstate (&t->attr, &detach_state );
		const char *state = (detach_state == (PTHREAD_CREATE_JOINABLE) ? "PTHREAD_CREATE_JOINABLE" : "PTHREAD_CREATE_DETACHED");
		if(show) {
			printf("Thread: %d Detach state: \t[ %s ]\tret: %d\n", t->id, state, ret);
			return 0;
		}

		lua_pushinteger(L, ret);
		lua_pushstring(L, state);

		return 2;
	}
	lua_pushnil(L);

	return 1;
};

static int thread_attr_scope(lua_State *L) {
	int nargs = lua_gettop(L);
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);
		const char *scope;
		int ret = -1, res = -1;

		if(nargs > 1) {
			/* set attribute scope to system */
			scope = luaL_checkstring(L, 2);
			if(strcmp(scope, "system") == 0) {
				ret = pthread_attr_setscope(&t->attr, PTHREAD_SCOPE_SYSTEM);
				//printf("Thread: %d Scope Set to [ PTHREAD_SCOPE_SYSTEM ] ret: %d\n", t->id, ret);
			}
			#ifndef __linux__ // linux doesn't allow process scope
			else
				ret = pthread_attr_setscope(&t->attr, PTHREAD_SCOPE_PROCESS);
			#endif
			lua_pushinteger(L, ret);

			return 1;
		}

		/* get attribute scope */
		ret = pthread_attr_getscope(&t->attr, &res);
		scope = (res == (PTHREAD_SCOPE_SYSTEM) ? "PTHREAD_SCOPE_SYSTEM" : "PTHREAD_SCOPE_PROCESS ");
		if(show) {
			printf("Thread: %d Scope: \t\t[ %s ]\tret: %d\n", t->id, scope, ret);
			return 0;
		}

		lua_pushinteger(L, ret);
		lua_pushstring(L, scope);

		return 2;
	}
	lua_pushnil(L);

	return 1;
};

static int thread_attr_inheritsched(lua_State *L) {
	int nargs = lua_gettop(L);
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);
		const char *inherit;
		int inherit_sched;
		int ret;

		if(nargs > 1) {
			/* set attribute scope to system */
			inherit = luaL_checkstring(L, 2);
			if(strcmp(inherit, "explicit") == 0) {
				/* set attribute scope to system */
				ret = pthread_attr_setinheritsched(&t->attr, PTHREAD_EXPLICIT_SCHED);
				//printf("Thread: %d Scope Set to [ PTHREAD_EXPLICIT_SCHED ] ret: %d\n", t->id, ret);
			}
			else {
				ret = pthread_attr_setinheritsched(&t->attr, PTHREAD_INHERIT_SCHED);
				//printf("Thread: %d [ PTHREAD_INHERIT_SCHED ] ret: %d\n", t->id, ret);
			}
			lua_pushinteger(L, ret);

			return 1;
		}

		/* get attribute scope */
		inherit = NULL;
		ret = pthread_attr_getinheritsched(&t->attr, &inherit_sched);
		inherit = (inherit_sched == (PTHREAD_INHERIT_SCHED) ? "PTHREAD_INHERIT_SCHED" : "PTHREAD_EXPLICIT_SCHED");
		if(show) {
			printf("Thread: %d Inherit Scheduler: \t[ %s ]\tret: %d\n", t->id, inherit, ret);
			return 0;
		}
		lua_pushinteger(L, ret);
		lua_pushstring(L, inherit);

		return 2;
	}
	lua_pushnil(L);

	return 1;
};

static int thread_attr_schedpolicy(lua_State *L) {
	int nargs = lua_gettop(L);
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);
		const char *policy = NULL;
		int sched_policy = 0;
		int ret = 0;

		if(nargs > 1) {
			policy = luaL_checkstring(L, 2);
			if(strcmp(policy, "fifo") == 0) {
				/* set attribute scope to system */
				ret = pthread_attr_setschedpolicy(&t->attr, SCHED_FIFO);
				//printf("Thread: %d Update Sched Policy [ SCHED_FIFO ] ret: %d\n", t->id, ret);
			}
			else if(strcmp(policy, "rr")) {
				ret = pthread_attr_setschedpolicy(&t->attr, SCHED_RR);
				//printf("Thread: %d Update Sched Policy [ SCHED_RR ] ret: %d\n", t->id, ret);
			}
			else {
				ret = pthread_attr_setschedpolicy(&t->attr, SCHED_OTHER);
				//printf("Thread: %d Update Sched Policy [ SCHED_OTHER ] ret: %d\n", t->id, ret);
			}
				lua_pushinteger(L, ret);

			return 1;
		}

		ret = pthread_attr_getschedpolicy(&t->attr, &sched_policy);
		switch (sched_policy) {
			case SCHED_OTHER: {
				policy = "SCHED_OTHER";
				break;
			}
			case SCHED_FIFO: {
				policy = "SCHED_FIFO";
				break;
			}

			case SCHED_RR: {
				policy = "SCHED_RR";
				break;
			}
			default:
				policy = "UNKNOWN";
				break;
		}
		if(show) {
			printf("Thread: %d Sched Policy: \t[ %s ]\t\t\tret: %d\n", t->id, policy, ret);
			return 0;
		}
		lua_pushinteger(L, ret);
		lua_pushstring(L, policy);

		return 2;
	}
	lua_pushnil(L);

	return 1;
};

static int thread_attr_schedparam(lua_State *L) {
	int nargs = lua_gettop(L);
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);
		int newprio = 0;
		struct sched_param *param = (struct sched_param*)lua_newuserdata(L, sizeof(struct sched_param)); // create a newudata
		memset(param, 0, sizeof(struct sched_param)); lua_pop(L, 1); // now pop it off the stack
		int ret = 0;

		if(nargs > 1) {
			/* set attribute schedparam(priority) */
			newprio = luaL_checkinteger(L, 2);
			param->sched_priority = newprio;
			ret = pthread_attr_setschedparam(&t->attr,(const struct sched_param*)param);
			//printf("Thread: %d New Sched Proiority [ %d ] ret: %d\n", t->id, newprio, ret);
			lua_pushinteger(L, ret);

			return 1;
		}

		/* get attribute schedparam(priority) */
		ret =  pthread_attr_getschedparam(&t->attr, param);
		nargs = param->sched_priority;
		if(show) {
			printf("Thread: %d Sched Proiority:\t[ PRIORITY %d ]\t\t\tret: %d\n", t->id, nargs, ret);
			return 0;
		}
		lua_pushinteger(L, ret);
		lua_pushinteger(L, nargs);

		return 2;
	}
	lua_pushnil(L);

	return 1;
};

static int thread_attr_stacksize(lua_State *L) {
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);
		size_t stack_size = 0;
		int ret = 1;
		/* get attribute stacksize */
		ret =  pthread_attr_getstacksize(&t->attr, &stack_size);
		if(show) {
			printf("Thread: %d Stack size:\t\t[ %zu bytes ]\t\t\tret: %d\n", t->id, stack_size, ret);
			return 0;
		}
		lua_pushinteger(L, ret);
		lua_pushnumber(L, stack_size);

		return 2;
	}
	lua_pushnil(L);

	return 1;
};

static int thread_attr_stackaddr(lua_State *L) {
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);
		int ret = 1;
		void *stack_addr;
		size_t stack_size = 0;

		/* get attribute stackbase */
		ret =  pthread_attr_getstack(&t->attr, &stack_addr, &stack_size);
		if(show) {
			printf("Thread: %d Stack address:\t[ userdata: 0x0092740 ]\t\tret: %d\n", t->id, ret);
			return 0;
		}
		lua_pushinteger(L, ret);
		lua_pushstring(L, "userdata: 0x0B92740 "); // dummy value

		return 2;
	}
	lua_pushnil(L);

	return 1;
};

static int thread_attr_show(lua_State *L) {
	//lua_settop(L, 1);
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);
		show = true;
		printf("\n#################### Thread: %d Attributes ####################\n\n", t->id);
		thread_attr_detachstate(L);
		thread_attr_scope(L);
		thread_attr_inheritsched(L);
		thread_attr_schedparam(L);
		thread_attr_schedpolicy(L);
		thread_attr_stacksize(L);
		thread_attr_stackaddr(L);
		show = false;
	}

	return 0;
}

static const struct luaL_reg thread_attr_lib_m [] = {
	//{"init", 		thread_attr_init		},
	{"destroy", 		thread_attr_destroy		},
	{"detach",		thread_attr_detachstate		},
	{"scope", 		thread_attr_scope		},
	{"sched_inherit", 	thread_attr_inheritsched	},
	{"sched_param",		thread_attr_schedparam		},
	{"sched_policy",	thread_attr_schedpolicy		},
	{"stacksize", 		thread_attr_stacksize		},
	{"stackaddr", 		thread_attr_stackaddr		},
	{"show", 		thread_attr_show		},
	{NULL, NULL}
};

void lua_threads_open_thread_attr(lua_State *L) {
	lua_newtable(L);
	luaL_register(L, NULL, thread_attr_lib_m);
	lua_setfield(L, -2, ATTR);
	// thread_attr
	luaL_newmetatable(L, "LuaBook.thread_attr");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);  /* pushes the metatable */
	lua_settable(L, -3);  /* metatable.__index = metatable */
	luaL_openlib(L, NULL, thread_attr_lib_m, 0);
	lua_pop(L, 1);
};