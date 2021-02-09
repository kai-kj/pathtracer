#ifndef LUA_WRAPPER_H
#define LUA_WRAPPER_H

#include <lua.h>

lua_State *create_script(char *fileName);
void run_script(lua_State *l, char *functionName);
void destroy_script(lua_State *l);

#endif