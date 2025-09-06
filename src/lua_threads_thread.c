#include "lua_threads_thread.h"

// MAIN THREAD LOCK
pthread_mutex_t main_lock = PTHREAD_MUTEX_INITIALIZER;

// JOIN && CREATE LOCKS
pthread_mutex_t thread_create_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t thread_join_lock = PTHREAD_MUTEX_INITIALIZER;

// NEW THREAD AND THREAD CHECK LOCKS
pthread_mutex_t thread_new_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t thread_check_lock = PTHREAD_MUTEX_INITIALIZER;

// THREAD STATS LOCK 
pthread_mutex_t thread_stats_lock = PTHREAD_MUTEX_INITIALIZER;

_Atomic unsigned int active_threads 	= 0;
_Atomic unsigned int yielded_threads 	= 0;
_Atomic unsigned int completed_threads 	= 0;
_Atomic unsigned int joined_threads 	= 0;
_Atomic unsigned int thread_errs 	= 0;

// lua thread object indexes
// tables and 1 based index
static int thread_lid = 1; 

// forward function proto's
static int thread_index(lua_State *L);
static int thread_new_index(lua_State *L);
static int thread_newt(lua_State * L);
static int thread_stats(state_t state);

void* fn_lua(void *args) {
	uint8_t fn_args = 1;
	threads_thread_t *t = (threads_thread_t*)args;
	if(t) {
		pthread_mutex_lock(&t->mutex);
			lua_getglobal(t->T, t->fn);
			lua_pushlightuserdata(t->T, t);
		pthread_mutex_unlock(&t->mutex);

		if(t->fn_data) {
			pthread_mutex_lock(&t->mutex);
				lua_pushlightuserdata(t->T, t->fn_data);
			pthread_mutex_unlock(&t->mutex);
			fn_args++;
		}
		
		t->run = true;
		t->state = WORKING;
		t->done = false;
		active_threads++;
		lua_pcall(t->T, fn_args, 1, 0);
		t->done = true;
		thread_stats(t->state);

		return (void *)0;
	}
	return (void *)1;
};

threads_thread_t *check_threads_thread(lua_State *L, int pos) {
	pthread_mutex_lock(&thread_check_lock);
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "ctx");
		pos = -1;
	}

	void *ud = luaL_checkudata(L, pos, "LuaBook.threads_thread");
	luaL_argcheck(L, ud != NULL, pos, "`threads_thread' expected");
	pthread_mutex_unlock(&thread_check_lock);

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

// called from main thread ALWAYS
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
		
		t->id = thread_lid++;
		t->T = lua_newthread(L);
		lua_pop(L, 1);

		sem_init(&t->sem, 1, 1);  		// init thread semaphore
		pthread_mutex_init(&t->mutex, NULL);	// init thread mutex
		sem_init(&t->joinable, 1, 1);	// init thread mutex
		pthread_attr_init(&t->attr);

	}

	return 1;
};

static int thread_newt(lua_State * L) {
	pthread_mutex_lock(&main_lock);
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
	pthread_mutex_unlock(&main_lock);

	return 1;
};

static int thread_create(lua_State * L) {
	pthread_mutex_lock(&thread_create_lock);
	threads_thread_t *t = check_threads_thread(L, 1);
	pthread_mutex_lock(&t->mutex);
		t->fn = luaL_checkstring(L, 2);
		t->fn_data = NULL;
	pthread_mutex_unlock(&t->mutex);

	// pass a pointer or udata in t->fn_data
	if(lua_islightuserdata(L, 3)){
		pthread_mutex_lock(&t->mutex);
			t->fn_data = (void*)lua_topointer(L, 3);
		pthread_mutex_unlock(&t->mutex);
	}
		
	// pass a pointer or udata in t->fn_data
	if(lua_istable(L, 3)) {
		lua_getfield(L, 3, "ctx");
		threads_thread_t *tt =  (void*)lua_topointer(L, -1);
		pthread_mutex_lock(&t->mutex);
			t->fn_data = (void*)tt;
		pthread_mutex_unlock(&t->mutex);
	}
	
	// create the pthread and pass the lua fn and udata
	if(pthread_create(&t->thread, &t->attr, fn_lua, t) != 0)
		fprintf(stderr, "Failed to Start Thread\n");

	lua_settop(L, 1);
	pthread_mutex_unlock(&thread_create_lock);
	return 1;
};

static int thread_join(lua_State *L) {
	pthread_mutex_lock(&thread_join_lock);
	threads_thread_t *t = check_threads_thread(L, 1);
	
	int ret = -1;
	ret = pthread_join(t->thread, NULL);

	if(ret != 0 )
		fprintf(stderr, "failed to join thread (id: %d)\n", t->id);
	
	lua_pushinteger(L, ret);
	pthread_mutex_unlock(&thread_join_lock);
	return 1;
};

static int thread_resume(lua_State *L) {
	threads_thread_t *t = check_threads_thread(L, 1);
	if(t->state == SUSPENDED) {
		t->state = WORKING;
		thread_stats(WORKING);
	}

	return 0;
};

static int thread_yield(lua_State *L) {
	if(lua_istable(L, 1) || lua_isuserdata(L, 1)) {
		threads_thread_t *t = check_threads_thread(L, 1);
		if(t->state == WORKING) {
			t->state = SUSPENDED;
			thread_stats(SUSPENDED);
		}
	}
	sched_yield();

	return 0;
};

static int thread_cancel(lua_State *L) {
	threads_thread_t *t = check_threads_thread(L, 1);

	pthread_mutex_lock(&main_lock);
	pthread_mutex_lock((&t->mutex));
		int ret = pthread_cancel(t->thread);
	pthread_mutex_unlock((&t->mutex));
	pthread_mutex_unlock(&main_lock);
	lua_pushinteger(L, ret);

	return 1;
};

static int thread_complete(lua_State *L) {
	threads_thread_t *t = check_threads_thread(L, 1);
	t->state = OK;
	t->run = false;
	thread_stats(OK);

	sched_yield();

	return 0;
};

static int thread_error(lua_State *L) {
	threads_thread_t *t = check_threads_thread(L, 1);
	t->state = ERROR;
	t->run = false;
	thread_stats(ERROR);

	return 0;
};

static int thread_state(lua_State *L) {
	threads_thread_t *t = check_threads_thread(L, 1);
	lua_pushinteger(L, t->state);
		
	return 1;
};

static int thread_run(lua_State *L) {
	threads_thread_t *t = check_threads_thread(L, 1);
	lua_pushboolean(L, t->run);

	return 1;
};

static int thread_id(lua_State *L) {
	threads_thread_t *t = check_threads_thread(L, 1);
	lua_pushinteger(L, t->id);

	return 1;
};

static int thread_tid(lua_State *L) {
	threads_thread_t *t = check_threads_thread(L, 1);
	pthread_mutex_lock((&t->mutex));
		lua_pushinteger(L, (size_t)pthread_self());
	pthread_mutex_unlock((&t->mutex));

	return 1;
};

static int thread_exit(lua_State *L) {
	threads_thread_t *t = check_threads_thread(L, 1);
	size_t ret = t->tid;
	pthread_exit((void*)ret);

	return 0;
};

static int thread_new_index(lua_State *L) {
	if(lua_istable(L, 1)) {
		const char *key = luaL_checkstring(L, 2);
		printf("KEY: %s CANNOT BE UPDATED !\n", key);
		lua_pop(L, 1);
	}

	return 0;
};

static int thread_index(lua_State *L) {
	if(lua_istable(L, 1)) {
		const char *key = luaL_checkstring(L, 2);
		lua_pop(L, 1);

		if(key && strcmp(key, "id") == 0 ) {
			thread_id(L);
		}
		else if(key && strcmp(key, "tid") == 0 ) {
			thread_tid(L);
		}
		else if(key && strcmp(key, "state") == 0 ) {
			thread_state(L);
		}
		else if(key && strcmp(key, "run") == 0 ) {
			thread_run(L);
		}
		else if(key && strcmp(key, "thread") == 0 ) {
			lua_getfield(L, 1, "ctx");
			thread_new(L);
		}
		else
			lua_pushnil(L);

		return 1;
	}
	lua_pushnil(L);

	return 1;
};

static int thread_stats(state_t state){
	int ret = -1;
	switch(state)
	{
		case START:
		{
			active_threads++;
			ret = 0;
			break;
		};
		case WORKING:
		{
			yielded_threads--;
			active_threads++;
			ret = 0;
			break;
		};
		case SUSPENDED:
		{

			yielded_threads++;
			active_threads--;
			ret = 0;
			break;
		}
		case OK:
		{
			if(active_threads > 0 ) {
				active_threads--;
				completed_threads++;
			}
			ret = 0;
			break;
		};
		case ERROR:
		{
			thread_errs++;
			ret = 0;
			break;
		}
		default:
		{
			ret = 1;
			break;
		}

	}
	return ret;
}

static int thread_gc(lua_State *L) {
	threads_thread_t *t = (threads_thread_t*)lua_topointer(L, 1);
	printf("REMOVING THREAD %d\n", t->id);
	sem_destroy(&t->sem);
	pthread_mutex_destroy(&t->mutex);

	return 0;
};

static const struct luaL_reg threads_thread_lib_f[] = {
	{ "resume",	thread_resume	},
	{ "yield",	thread_yield	},
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
	// open thread buf module
	lua_threads_open_thread_buf(L);
	// open thread queue module
	lua_threads_open_thread_queue(L);

	lua_pop(L, 1);
};
