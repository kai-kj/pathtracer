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

int main(int argc, char *argv[]) {
	if(argc < 2 || argc > 3) {
		msg("argument error\n");
		return -1;
	}

	lua_State *l = create_script(argv[1]);

	if(argc == 3)
		run_script(l, argv[2]);
	else
		run_script(l, "main");
	
	destroy_script(l);

	return 0;
}
