cmake_minimum_required(VERSION 3.1...3.27)

#-------------------------------------------------------------------------------
# Symlink Macro
# usage: install_symlink(filepath sympath) -- for install only
#-------------------------------------------------------------------------------
macro(install_symlink filepath sympath)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${filepath} ${sympath})")
    install(CODE "message(\"-- Created symlink: ${sympath} -> ${filepath}\")")
endmacro(install_symlink)

#-------------------------------------------------------------------------------
# Build Environment
#-------------------------------------------------------------------------------

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  add_definitions(-fpic -shared -O3)
  add_compile_options(-W -Wall -Werror -Wextra)
endif()

project(
  lua_threads
  VERSION 1.0
  LANGUAGES C
  )

set(CMAKE_SHARED_LIBRARY_PREFIX "")

add_library(threads SHARED lua_threads.c lua_threads_thread.c lua_threads_mutex.c lua_threads_sem.c lua_threads_attr.c lua_threads_cond.c
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

install(TARGETS threads  DESTINATION /usr/lib/lua/lua_threads/)
install(DIRECTORY src/examples DESTINATION dir/usr/lib/lua/lua_threads/examples/)
install_symlink(threads src/examples/)
