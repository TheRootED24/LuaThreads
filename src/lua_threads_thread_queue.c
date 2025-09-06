#include "lua_threads_thread_queue.h"

static int thread_queue_index(lua_State *L) ;
static int thread_queue_new_index(lua_State *L);
static int thread_queue_gc(lua_State *L);

static int thread_queue_new(lua_State *L) {
        pthread_mutex_t *m = (pthread_mutex_t*)lua_newuserdata(L, sizeof(pthread_mutex_t));
        memset(m, 0, sizeof(pthread_mutex_t));
        pthread_mutex_init(m, NULL);

        lua_newtable(L);
	    lua_pushvalue(L, 1);
	    lua_setfield(L, -2, "mutex");

        pthread_mutex_t *m2 = (pthread_mutex_t*)lua_newuserdata(L, sizeof(pthread_mutex_t));
        memset(m2, 0, sizeof(pthread_mutex_t));
        pthread_mutex_init(m2, NULL);
        lua_setfield(L, -2, "cond_mutex");

        pthread_cond_t *r = (pthread_cond_t*)lua_newuserdata(L, sizeof(pthread_cond_t));
        memset(r, 0, sizeof(pthread_cond_t));
        pthread_cond_init(r, NULL);
        lua_setfield(L, -2, "read_cond");

        pthread_cond_t *w = (pthread_cond_t*)lua_newuserdata(L, sizeof(pthread_cond_t));
        memset(w, 0, sizeof(pthread_cond_t));
        pthread_cond_init(w, NULL);
        lua_setfield(L, -2, "write_cond");

        luaL_getmetatable(L, "LuaBook.thread_queue");
        lua_setmetatable(L, -2);

    return 1;
}



/*static int thread_queue_index(lua_State *L) {
    if(lua_istable(L, 1)) {
        lua_getfield(L, 1, "mutex");
        pthread_mutex_t *lock = (pthread_mutex_t *)lua_topointer(L, -1);
        lua_pop(L, 1);

        pthread_mutex_lock(lock);
        const char *key = luaL_checkstring(L, 2);
        lua_getfield(L, 1, key);

        return 1;
    }
    lua_pushnil(L);
    return 1;

}

static int thread_queue_new_index(lua_State *L) {
     if(lua_istable(L, 1)) {
        lua_getfield(L, 1, "mutex");
        pthread_mutex_t *lock = (pthread_mutex_t *)lua_topointer(L, -1);
        lua_pop(L, 1);

        pthread_mutex_lock(lock);
        const char *key = luaL_checkstring(L, 2);
        lua_getfield(L, 1, key);

        return 1;
    }
    lua_pushnil(L);
    return 1;

}*/

static int thread_queue_index(lua_State *L) {
    if(lua_istable(L, 1)) {
        lua_getfield(L, 1, "mutex");
        pthread_mutex_t *m = (pthread_mutex_t *)lua_topointer(L, -1);
        lua_pop(L, 1);

        pthread_mutex_lock(m);
            const char *key = luaL_checkstring(L, 2);
            lua_getfield(L, 1, key);
            lua_getfield(L, 1, "read_cond");
            pthread_cond_broadcast((pthread_cond_t*)lua_topointer(L, -1));
            lua_pop(L, 1);
        pthread_mutex_unlock(m);

        return 1;
    }
    lua_pushnil(L);

    return 1;
}

static int thread_queue_new_index(lua_State *L) {
    if(lua_istable(L, 1)) {
        lua_getfield(L, 1, "mutex");
        pthread_mutex_t *m = (pthread_mutex_t *)lua_topointer(L, -1);
        lua_pop(L, 1);

        pthread_mutex_lock(m);
        const char *key = luaL_checkstring(L, 2);
            if(strcmp(key, "mutex") != 0 && strcmp(key, "write_cond") != 0 && strcmp(key, "read_cond") != 0)
                lua_rawset(L, 1);
        lua_getfield(L, 1, "write_cond");
        pthread_cond_broadcast((pthread_cond_t*)lua_topointer(L, -1));
        lua_pop(L, 1);
        pthread_mutex_unlock(m);

        return 0;
    }
    return 0;
}

static int thread_queue_gc(lua_State *L) {
    if(lua_istable(L, 1)) {
    lua_getfield(L, 1, "mutex");
    pthread_mutex_t *m = (pthread_mutex_t *)lua_topointer(L, -1);
    lua_pop(L, 1);
    pthread_mutex_destroy(m);

    lua_getfield(L, 1, "read_cond");
    pthread_cond_destroy((pthread_cond_t*)lua_topointer(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, 1, "write_cond");
    pthread_cond_destroy((pthread_cond_t*)lua_topointer(L, -1));
    lua_pop(L, 1);

    printf("********************** Queue Destroyed **************************\n");

    }

    return 0;
}
static const struct luaL_reg thread_queue_lib_m [] = {
	{"new", 	    thread_queue_new		},
	{NULL, NULL}
};

void lua_threads_open_thread_queue(lua_State *L) {
	lua_newtable(L);
	luaL_register(L, NULL, thread_queue_lib_m);
	lua_setfield(L, -2, QUEUE);
	// thread_sem
	luaL_newmetatable(L, "LuaBook.thread_queue");
	lua_pushstring(L, "__index");
	lua_pushcfunction(L, thread_queue_index);
	lua_settable(L, -3); // set the __index in the metatable (-3)
	lua_pushstring(L, "__newindex");
	lua_pushcfunction(L, thread_queue_new_index);
	lua_settable(L, -3); // set the __newindex in the metatable (-3)
    lua_pushstring(L, "__gc");
	lua_pushcfunction(L, thread_queue_gc);
	lua_settable(L, -3); // set the __newindex in the metatable (-3)
	luaL_openlib(L, NULL, thread_queue_lib_m, 0);
	lua_pop(L, 1);
};