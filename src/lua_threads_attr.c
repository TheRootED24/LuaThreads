#include "lua_threads_attr.h"
pthread_mutex_t attr_mutex = PTHREAD_MUTEX_INITIALIZER;

static bool show = false;

static int thread_attr_destroy(lua_State *L) {
	int ret = -1;
	threads_thread_t *t = check_threads_thread(L, 1);

	/* destroy initialized attribute */
	ret = pthread_attr_destroy(&t->attr);
	lua_pushinteger(L, ret);

	return 1;
};

static int thread_attr_detachstate(lua_State *L) {
	int nargs = lua_gettop(L);
	threads_thread_t *t = check_threads_thread(L, 1);
	int ret = -1, detach_state = -1;

	if(!show && nargs > 1) {
		/* set attribute detachedstate */
		pthread_mutex_lock(&t->mutex);
			ret = pthread_attr_setdetachstate(&t->attr, PTHREAD_CREATE_DETACHED);
		pthread_mutex_unlock(&t->mutex);

		lua_pushinteger(L, ret);

		return 1;
	}

	/* get attribute detachedstate */
	int id = 0;
	pthread_mutex_lock(&t->mutex);
		ret = pthread_attr_getdetachstate (&t->attr, &detach_state );
		id = t->id;
	pthread_mutex_unlock(&t->mutex);

	const char *state = (detach_state == (PTHREAD_CREATE_JOINABLE) ? "PTHREAD_CREATE_JOINABLE" : "PTHREAD_CREATE_DETACHED");
	if(show) {
		printf("Thread: %d Detach state: \t[ %s ]\tret: %d\n", id, state, ret);
		return 0;
	}

	lua_pushinteger(L, ret);
	lua_pushstring(L, state);

	return 2;
};

static int thread_attr_scope(lua_State *L) {
	int nargs = lua_gettop(L);
	threads_thread_t *t = check_threads_thread(L, 1);
	const char *scope;
	int ret = -1, res = -1;

	if(!show && nargs > 1) {
		/* set attribute scope to system */
		scope = luaL_checkstring(L, 2);
		if(strcmp(scope, "system") == 0) {
			pthread_mutex_lock(&t->mutex);
				ret = pthread_attr_setscope(&t->attr, PTHREAD_SCOPE_SYSTEM);
			pthread_mutex_unlock(&t->mutex);
		}
		#ifndef __linux__ // linux doesn't allow process scope
		else
			ret = pthread_attr_setscope(&t->attr, PTHREAD_SCOPE_PROCESS);
		#endif
		lua_pushinteger(L, ret);

		return 1;
	}

	/* get attribute scope */
	int id = 0;
	pthread_mutex_lock(&t->mutex);
		ret = pthread_attr_getscope(&t->attr, &res);
		id = t->id;
	pthread_mutex_unlock(&t->mutex);

	scope = (res == (PTHREAD_SCOPE_SYSTEM) ? "PTHREAD_SCOPE_SYSTEM" : "PTHREAD_SCOPE_PROCESS ");
	if(show) {
		printf("Thread: %d Scope: \t\t[ %s ]\tret: %d\n", id, scope, ret);
		return 0;
	}

	lua_pushinteger(L, ret);
	lua_pushstring(L, scope);

	return 2;
};

static int thread_attr_inheritsched(lua_State *L) {
	int nargs = lua_gettop(L);
	threads_thread_t *t = check_threads_thread(L, 1);
	const char *inherit;
	int inherit_sched;
	int ret;

	if(!show && nargs > 1) {
		/* set attribute inheritsched */
		inherit = luaL_checkstring(L, 2);
		if(strcmp(inherit, "explicit") == 0) {
			/* set attribute inheritsched to explicit */
			pthread_mutex_lock(&t->mutex);
				ret = pthread_attr_setinheritsched(&t->attr, PTHREAD_EXPLICIT_SCHED);
			pthread_mutex_unlock(&t->mutex);
		}
		else {
			/* set attribute inheritsched to inherit */
			pthread_mutex_lock(&t->mutex);
				ret = pthread_attr_setinheritsched(&t->attr, PTHREAD_INHERIT_SCHED);
			pthread_mutex_unlock(&t->mutex);
		}
		lua_pushinteger(L, ret);

		return 1;
	}

	/* get attribute inheritsched */
	inherit = NULL;
	int id = 0;

	pthread_mutex_lock(&t->mutex);
		ret = pthread_attr_getinheritsched(&t->attr, &inherit_sched);
		id = t->id;
	pthread_mutex_unlock(&t->mutex);

	inherit = (inherit_sched == (PTHREAD_INHERIT_SCHED) ? "PTHREAD_INHERIT_SCHED" : "PTHREAD_EXPLICIT_SCHED");
	if(show) {
		printf("Thread: %d Inherit Scheduler: \t[ %s ]\tret: %d\n", id, inherit, ret);

		return 0;
	}
	lua_pushinteger(L, ret);
	lua_pushstring(L, inherit);

	return 2;
};

static int thread_attr_schedpolicy(lua_State *L) {
	int nargs = lua_gettop(L);
	threads_thread_t *t = check_threads_thread(L, 1);
	const char *policy = NULL;
	int sched_policy = 0;
	int ret = 0;

	if(!show && nargs > 1) {
		policy = luaL_checkstring(L, 2);
		if(strcmp(policy, "fifo") == 0) {
			/* set attribute schedpolicy to fifo */
			pthread_mutex_lock(&t->mutex);
				ret = pthread_attr_setschedpolicy(&t->attr, SCHED_FIFO);
			pthread_mutex_unlock(&t->mutex);
		}
		else if(strcmp(policy, "rr")) {
			/* set attribute schedpolicy to round robin */
			pthread_mutex_lock(&t->mutex);
				ret = pthread_attr_setschedpolicy(&t->attr, SCHED_RR);
			pthread_mutex_unlock(&t->mutex);
		}
		else {
			/* set attribute schedpolicy to other */
			pthread_mutex_lock(&t->mutex);
				ret = pthread_attr_setschedpolicy(&t->attr, SCHED_OTHER);
			pthread_mutex_unlock(&t->mutex);
		}
			lua_pushinteger(L, ret);

		return 1;
	}

	/* get attribute schedpolicy */
	int id = 0;
	pthread_mutex_lock(&t->mutex);
		ret = pthread_attr_getschedpolicy(&t->attr, &sched_policy);
		id = t->id;
	pthread_mutex_unlock(&t->mutex);

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
		printf("Thread: %d Sched Policy: \t[ %s ]\t\t\tret: %d\n", id, policy, ret);

		return 0;
	}
	lua_pushinteger(L, ret);
	lua_pushstring(L, policy);

	return 2;
};

static int thread_attr_schedparam(lua_State *L) {
	int nargs = lua_gettop(L);
	threads_thread_t *t = check_threads_thread(L, -1);
	int newprio = 0;
	struct sched_param *param = (struct sched_param*)lua_newuserdata(L, sizeof(struct sched_param)); // create a newudata
	memset(param, 0, sizeof(struct sched_param)); lua_pop(L, 1); // now pop it off the stack
	int ret = 0;

	if(!show && nargs > 1) {
		/* set attribute schedparam(priority) */
		newprio = luaL_checkinteger(L, 2);
		param->sched_priority = newprio;
		pthread_mutex_lock(&t->mutex);
			ret = pthread_attr_setschedparam(&t->attr,(const struct sched_param*)param);
		pthread_mutex_unlock(&t->mutex);
		lua_pushinteger(L, ret);

		return 1;
	}

	/* get attribute schedparam(priority) */
	int id = 0;
	pthread_mutex_lock(&t->mutex);
		ret =  pthread_attr_getschedparam(&t->attr, param);
		id = t->id;
	pthread_mutex_unlock(&t->mutex);

	nargs = param->sched_priority;
	if(show) {
		printf("Thread: %d Schedparam:\t[ PRIORITY %d ]\t\t\tret: %d\n", id, nargs, ret);

		return 0;
	}
	lua_pushinteger(L, ret);
	lua_pushinteger(L, nargs);

	return 2;
};

static int thread_attr_stacksize(lua_State *L) {
	threads_thread_t *t = check_threads_thread(L, -1);
	size_t stack_size = 0;
	int ret = 1, id = 0;
	/* get attribute stacksize */
	pthread_mutex_lock(&t->mutex);
		ret =  pthread_attr_getstacksize(&t->attr, &stack_size);
		id = t->id;
	pthread_mutex_unlock(&t->mutex);

	if(show) {
		printf("Thread: %d Stack size:\t\t[ %zu bytes ]\t\t\tret: %d\n", id, stack_size, ret);

		return 0;
	}
	lua_pushinteger(L, ret);
	lua_pushnumber(L, stack_size);

	return 2;
};

static int thread_attr_stackaddr(lua_State *L) {
	threads_thread_t *t = check_threads_thread(L, -1);
	int ret = 1, id = 0;
	void *stack_addr;
	size_t stack_size = 0;

	/* get attribute stackbase */
	pthread_mutex_lock(&t->mutex);
		ret =  pthread_attr_getstack(&t->attr, &stack_addr, &stack_size);
		id = t->id;
	pthread_mutex_unlock(&t->mutex);
	if(show) {
		printf("Thread: %d Stack address:\t[ userdata: %p ]\tret: %d\n", id, stack_addr, ret);

		return 0;
	}
	lua_pushinteger(L, ret);
	lua_pushlightuserdata(L, stack_addr);

	return 2;
};

static int thread_attr_show(lua_State *L) {
	pthread_mutex_lock(&attr_mutex);
	threads_thread_t *t = check_threads_thread(L, 1);
	
	pthread_mutex_lock(&t->mutex);
		printf("\n#################### Thread: %d Attributes ####################\n\n", t->id);
	pthread_mutex_unlock(&t->mutex);

	show = true;
	thread_attr_detachstate(L);
	thread_attr_scope(L);
	thread_attr_inheritsched(L);
	thread_attr_schedparam(L);
	thread_attr_schedpolicy(L);
	thread_attr_stacksize(L);
	thread_attr_stackaddr(L);
	show = false;
	pthread_mutex_unlock(&attr_mutex);
	
	return 0;
}

static const struct luaL_reg thread_attr_lib_m [] = {
	//{"init", 		thread_attr_init		},
	{"destroy", 		thread_attr_destroy		},
	{"detachstate",		thread_attr_detachstate		},
	{"scope", 		thread_attr_scope		},
	{"inheritsched", 	thread_attr_inheritsched	},
	{"schedparam",		thread_attr_schedparam		},
	{"schedpolicy",		thread_attr_schedpolicy		},
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

	pthread_mutex_init(&attr_mutex, NULL);
};