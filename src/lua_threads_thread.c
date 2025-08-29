#include "lua_threads_thread.h"

pthread_mutex_t main_lock;
sem_t main_sem_lock;
sem_t sem_lock;

static uint8_t threads[MAX_THREADS] = {0};
static int thread_lid = 0;

int active_threads 	= 0;
int yielded_threads 	= 0;
int completed_threads 	= 0;
int joined_threads 	= 0;
int thread_errs 	= 0;

// forward function proto's
static int thread_index(lua_State *L);
static int thread_new_index(lua_State *L);
static int thread_newt(lua_State * L);

void* fn_lua(void *args) {
	uint8_t fn_args = 0;
	threads_thread_t *t = (threads_thread_t*)args;
	if(t) {
		lua_getglobal(t->T, t->fn);
		lua_pushlightuserdata(t->T, t);
		fn_args++;

		if(t->fn_data) {
			lua_pushlightuserdata(t->T, t->fn_data);
			fn_args++;
		}

		lua_pcall(t->T, fn_args, 1, 0);
		return (void *)0;
	}
	return (void *)1;
};

threads_thread_t *check_threads_thread(lua_State *L, int pos) {
	void *ud = luaL_checkudata(L, pos, "LuaBook.threads_thread");
	luaL_argcheck(L, ud != NULL, pos, "`threads_thread' expected");

	return (threads_thread_t*)ud;
};

static int thread_sleep(lua_State *L) {
	struct timespec req_time;
	struct timespec rem_time;

	time_t t = (time_t)luaL_optlong(L, 1, 0);
	long nano = luaL_optlong(L, 2, 0);

	req_time.tv_sec = t;
	req_time.tv_nsec = nano;

	if (nanosleep(&req_time, &rem_time) == -1)
		perror("nanosleep");

	return 0;
};

static int next_thread() {
	if(active_threads < MAX_THREADS) {
          return thread_lid++;
        }

	fprintf(stderr, "No thread slot avaiable ( available %d | active %d)\n", MAX_THREADS, active_threads);
	return -1;
};

static int thread_new(lua_State * L) {
	threads_thread_t *t = NULL;

	int nargs = lua_gettop(L);
	if(nargs > 0)
		t = (threads_thread_t*)lua_touserdata(L, 1);
	else {
		t = (threads_thread_t*)lua_newuserdata(L, sizeof(threads_thread_t));
		memset(t, 0, sizeof(threads_thread_t));
	}

	luaL_getmetatable(L, "LuaBook.threads_thread");
	lua_setmetatable(L, -2);

	if(nargs == 0) {
		t->id = next_thread();
		sem_init(&t->sem, 1, 1);  		// init thread semaphore
		pthread_mutex_init(&t->mutex, NULL);	// init thread mutex
		t->T = lua_newthread(L);  		// create lua thread
		pthread_attr_init(&t->attr);
		lua_pop(L, 1);
		t->TS = luaL_newstate();
		void *stack_addr = lua_newuserdata(t->TS, STACK_SIZE);
		pthread_attr_setstack(&t->attr, stack_addr, STACK_SIZE);
                int res = pthread_attr_setschedpolicy(&t->attr, SCHED_FIFO);
                int newprio = 0;
		struct sched_param *param = (struct sched_param*)lua_newuserdata(L, sizeof(struct sched_param)); // create a newudata
		memset(param, 0, sizeof(struct sched_param)); lua_pop(L, 1); // now pop it off the stack
                param->sched_priority = t->id;
		res = pthread_attr_setschedparam(&t->attr,(const struct sched_param*)param);
	}

	return 1;
};

static int thread_newt(lua_State * L) {
	thread_new(L);

	lua_newtable(L);
	lua_pushvalue(L, 1);
	lua_setfield(L, -2, "ctx");

	lua_newtable(L);
	lua_pushstring(L, "__index");
	lua_pushcfunction(L, thread_index);
	lua_settable(L, -3); // set the __index in the metatable (-3)

	lua_pushstring(L, "__newindex");
	lua_pushcfunction(L, thread_new_index);
	lua_settable(L, -3); // set the __newindex in the metatable (-3)

	lua_setmetatable(L, -2);

	return 1;
};

static int thread_create(lua_State * L) {
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);
		lua_pop(L, 1);

		t->fn = luaL_checkstring(L, 2);
		t->state = WORKING;
		t->run = true;
		t->fn_data = NULL;

		// pass a pointer or udata in t->fn_data
		if(lua_islightuserdata(L, 3) || lua_isuserdata(L, 3))
			t->fn_data = (void*)lua_topointer(L, 3);

		// pass a pointer or udata in t->fn_data
		if(lua_istable(L, 3)) {
			lua_getfield(L, 3, "ctx");
			threads_thread_t *tt = check_threads_thread(L, -1);

			t->fn_data = (void*)tt;

		}

		// create the pthread and pass the lua fn and udata
		if(pthread_create(&t->thread, &t->attr, fn_lua, t) != 0)
			fprintf(stderr, "Failed to Start Thread\n");
	}
	lua_settop(L, 1);

	return 1;
};

/*static int thread_create(lua_State * L) {
	threads_thread_t *t = check_threads_thread(L, 1);
	t->fn = luaL_checkstring(L, 2);
	t->state = WORKING;
	t->run = true;
	t->fn_data = NULL;

	// pass a pointer or udata in t->fn_data
	if(lua_islightuserdata(L, 3) == 1 || lua_isuserdata(L, 3))
		t->fn_data = (void*)lua_topointer(L, 3);

	// create the pthread and pass the lua cb and udata
	if(pthread_create(&t->thread, NULL, fn_lua, t) != 0)
		fprintf(stderr, "Failed to Start Thread\n");

	lua_settop(L, 1);
	return 1;
};*/

static int thread_join(lua_State *L) {
	int ret = 1;
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);

		bool force;
		if(lua_isboolean(L, 2))
			force = true;

		if(force && t->run) {
			pthread_mutex_lock(&main_lock);
				t->run = false;
				if(active_threads > 0) {
					--active_threads;
					++completed_threads;
				}
			pthread_mutex_unlock(&main_lock);
		}
		// destroy the pthread
		void *res = NULL;
		if(t->thread)
			ret = pthread_join(t->thread, res);

		if(ret != 0 )
			fprintf(stderr, "failed to join thread (id: %d)\n", t->id);

		if(ret == 0) {
			sem_destroy(&t->sem);
			pthread_mutex_destroy(&t->mutex);
			t->T = NULL;
			threads[t->id] = 0;
		}
	}
	lua_pushinteger(L, ret);

	return 1;
};

static int thread_resume(lua_State *L) {
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);
		pthread_mutex_lock((&t->mutex));
			t->state = WORKING;
		pthread_mutex_unlock((&t->mutex));
		pthread_mutex_lock(&main_lock);
			++active_threads;
			--yielded_threads;
		pthread_mutex_unlock(&main_lock);
	}

	return 0;
};

static int thread_yield(lua_State *L) {
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);
		pthread_mutex_lock((&t->mutex));
			t->state = SUSPENDED;
		pthread_mutex_unlock((&t->mutex));
		pthread_mutex_lock(&main_lock);
			--active_threads;
			++yielded_threads;
		pthread_mutex_unlock(&main_lock);
	}
	sched_yield();

	return 0;
};

static int thread_cancel(lua_State *L) {
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);

		pthread_mutex_lock((&t->mutex));
			int ret = pthread_cancel(t->thread);
		pthread_mutex_unlock((&t->mutex));
		lua_pushinteger(L, ret);

		return 1;
	}
	lua_pushnil(L);

	return 1;
};

static int thread_complete(lua_State *L) {
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);
		pthread_mutex_lock((&t->mutex));
			t->state = OK;
			t->run = false;
		pthread_mutex_unlock((&t->mutex));
		if(active_threads > 0) {
			pthread_mutex_lock(&main_lock);
				--active_threads;
				++completed_threads;
			pthread_mutex_unlock(&main_lock);
		}
	}
	sched_yield();

	return 0;
};

static int thread_error(lua_State *L) {
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);
		pthread_mutex_lock((&t->mutex));
			t->state = ERROR;
			t->run = false;
		pthread_mutex_unlock((&t->mutex));
		pthread_mutex_lock(&main_lock);
			++thread_errs;
		pthread_mutex_unlock(&main_lock);
	}

	return 0;
};

static int thread_state(lua_State *L) {
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);
		pthread_mutex_lock((&t->mutex));
			lua_pushinteger(L, t->state);
		pthread_mutex_unlock((&t->mutex));

		return 1;
	}
	lua_pushnil(L);

	return 1;
};

static int thread_run(lua_State *L) {
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);
		pthread_mutex_lock((&t->mutex));
			lua_pushboolean(L, t->run);
		pthread_mutex_unlock((&t->mutex));

		return 1;
	}
	lua_pushnil(L);

	return 1;
};

static int thread_id(lua_State *L) {
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);
		pthread_mutex_lock((&t->mutex));
			lua_pushinteger(L, t->id);
		pthread_mutex_unlock((&t->mutex));

		return 1;
	}
	lua_pushnil(L);

	return 1;
};

static int thread_tid(lua_State *L) {
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);
		pthread_mutex_lock((&t->mutex));
			lua_pushinteger(L, (size_t)pthread_self());
		pthread_mutex_unlock((&t->mutex));

		return 1;
	}
	lua_pushnil(L);

	return 1;
};

static int thread_sem(lua_State *L) {
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);
		pthread_mutex_lock(&main_lock);
			lua_pushlightuserdata(L, &t->sem);
		pthread_mutex_unlock(&main_lock);

		return 1;
	}
	lua_pushnil(L);

	return 1;
};

static int thread_mutex(lua_State *L) {
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);
		pthread_mutex_lock(&main_lock);
			lua_pushlightuserdata(L, &t->mutex);
		pthread_mutex_unlock(&main_lock);

		return 1;
	}
	lua_pushnil(L);

	return 1;
};

static int thread_exit(lua_State *L) {
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		threads_thread_t *t = check_threads_thread(L, -1);
		size_t ret = t->tid;
		pthread_exit((void*)ret);
	}
	else
		pthread_exit(NULL);

	return 1;
};

static int thread_new_index(lua_State *L) {
	if(lua_istable(L, 1)) {
		const char *key = luaL_checkstring(L, 2);
		printf("KEY: %s\n", key);
		lua_pop(L, 1);

	}

	return 0;
};

static int thread_index(lua_State *L) {
	if(lua_istable(L, 1)) {
		const char *key = luaL_checkstring(L, 2);
		lua_pop(L, 1);

		if(strcmp(key, "id") == 0 ) {
			thread_id(L);
		}
		else if(strcmp(key, "tid") == 0 ) {
			thread_tid(L);
		}
		else if(strcmp(key, "state") == 0 ) {
			thread_state(L);
		}
		else if(strcmp(key, "run") == 0 ) {
			thread_run(L);
		}
		else if(strcmp(key, "mutex") == 0 ) {
			thread_mutex(L);
		}
		else if(strcmp(key, "sem") == 0 ) {
			thread_sem(L);
		}
		else
			lua_pushnil(L);

		return 1;
	}
	lua_pushnil(L);

	return 1;
};


static int thread_gc(lua_State *L) {
	threads_thread_t *t = (threads_thread_t*)lua_topointer(L, 1);
	//printf("REMOVING THREAD %d\n", t->id);
	sem_destroy(&t->sem);
	pthread_mutex_destroy(&t->mutex);

	sem_destroy(&main_sem_lock);
	pthread_mutex_destroy(&main_lock);
	lua_close(t->TS);

	return 0;
};

static const struct luaL_reg threads_thread_lib_f[] = {
	{ "resume",	thread_resume	},
	{ "yield",	thread_yield	},
	{ "mutex",	thread_mutex,	},
	{ "sem",	thread_sem,	},
	{ "complete",	thread_complete	},
	{ "run", 	thread_run	},
	{ "id",		thread_id	},
	{ "state", 	thread_state	},
	{ "sleep",	thread_sleep	},
	{ "errors",	thread_error	},
	{ "exit",	thread_exit	},
	{NULL, NULL}
};

static const struct luaL_reg threads_thread_lib_m[] = {
	{ "new_t",	thread_new	},
	{ "new",	thread_newt	},
	{ "create",	thread_create	},
	{ "resume",	thread_resume	},
	{ "yield",	thread_yield	},
	{ "mutex",	thread_mutex,	},
	{ "sem",	thread_sem,	},
	{ "complete",	thread_complete	},
	{ "run", 	thread_run	},
	{ "join",	thread_join	},
	{ "cancel",	thread_cancel	},
	{ "id",		thread_id	},
	{ "tid",	thread_tid	},
	{ "state", 	thread_state	},
	{ "sleep",	thread_sleep	},
	{ "exit",	thread_exit	},
	{NULL, NULL}
};

void lua_threads_open_threads_thread (lua_State *L) {
	lua_newtable(L);
	luaL_register(L, NULL, threads_thread_lib_m);
	lua_setfield(L, -2, THREAD);
	// threads_thread
	luaL_newmetatable(L, "LuaBook.threads_thread");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);  /* pushes the metatable */
	lua_settable(L, -3);  /* metatable.__index = metatable */
	lua_pushcfunction(L, thread_gc);
	lua_setfield(L, -2, "__gc");

	luaL_openlib(L, NULL, threads_thread_lib_m, 0);
	luaL_openlib(L, "threads_thread", threads_thread_lib_f, 0);
	lua_pop(L, 2);

	// open thread mutex module
	lua_threads_open_thread_mutex(L);
	// open thread_sem module
	lua_threads_open_thread_sem(L);
	// open thread cond module
	lua_threads_open_thread_cond(L);
	// open thread attr module
	lua_threads_open_thread_attr(L);

	lua_pop(L, 1);
};
