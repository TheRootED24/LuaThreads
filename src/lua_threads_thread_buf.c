#include "lua_threads_thread_buf.h"

static int thread_buf_index(lua_State *L) ;
static int thread_buf_new_index(lua_State *L);
//static int thread_buf_gc(lua_State *L);

static int thread_buf_new(lua_State *L) {
        pthread_mutex_t *m = (pthread_mutex_t*)lua_newuserdata(L, sizeof(pthread_mutex_t));
        memset(m, 0, sizeof(pthread_mutex_t));
        pthread_mutex_init(m, NULL);

        lua_newtable(L);
	    lua_pushvalue(L, 1);
	    lua_setfield(L, -2, "mutex");

        luaL_getmetatable(L, "LuaBook.thread_buf");
        lua_setmetatable(L, -2);

    return 1;
}



/*static int thread_buf_index(lua_State *L) {
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

static int thread_buf_new_index(lua_State *L) {
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

static int thread_buf_index(lua_State *L) {
    if(lua_istable(L, 1)) {
        lua_getfield(L, 1, "mutex");
        pthread_mutex_t *m = (pthread_mutex_t *)lua_topointer(L, -1);
        lua_pop(L, 1);

        pthread_mutex_lock(m);
            const char *key = luaL_checkstring(L, 2);
            lua_getfield(L, 1, key);
        pthread_mutex_unlock(m);

        return 1;
    }
    lua_pushnil(L);

    return 1;
}

static int thread_buf_new_index(lua_State *L) {
    if(lua_istable(L, 1)) {
        lua_getfield(L, 1, "mutex");
        pthread_mutex_t *m = (pthread_mutex_t *)lua_topointer(L, -1);
        lua_pop(L, 1);

        pthread_mutex_lock(m);
        const char *key = luaL_checkstring(L, 2);
            if(strcmp(key, "mutex") != 0)
                lua_rawset(L, 1);

        pthread_mutex_unlock(m);

        return 0;
    }
    return 0;
}

/*static int thread_buf_gc(lua_State *L) {
	printf("MUTEX DESTROYED !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	if(lua_istable(L, 1)) {
		lua_getfield(L, 1, "mutex");
		pthread_mutex_t *m = (pthread_mutex_t *)lua_topointer(L, -1);
		lua_pop(L, 1);
		
		pthread_mutex_destroy(m);
	}

    	return 0;
}*/

static const struct luaL_reg thread_buf_lib_m [] = {
	{"new", 	    thread_buf_new		},
	{NULL, NULL}
};

void lua_threads_open_thread_buf(lua_State *L) {
	lua_newtable(L);
	luaL_register(L, NULL, thread_buf_lib_m);
	lua_setfield(L, -2, BUF);
	// thread_sem
	luaL_newmetatable(L, "LuaBook.thread_buf");
	lua_pushstring(L, "__index");
	lua_pushcfunction(L, thread_buf_index);
	lua_settable(L, -3); // set the __index in the metatable (-3)
	lua_pushstring(L, "__newindex");
	lua_pushcfunction(L, thread_buf_new_index);
	lua_settable(L, -3); // set the __newindex in the metatable (-3)
    	
	luaL_openlib(L, NULL, thread_buf_lib_m, 0);
	lua_pop(L, 1);
};