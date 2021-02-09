#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <CL/cl.h>

#define K_UTIL_IMPLEMENTATION
#include "../../include/k_util.h"

#include "lua_wrapper.h"

int main(void) {
	lua_State *l = create_script("scripts/main.lua");
	run_script(l, "main");
	destroy_script(l);

	return 0;
}
