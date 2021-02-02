//============================================================================//
// k_image.h                                                                //
//============================================================================//

//----------------------------------------------------------------------------//
// interface                                                                  //
//----------------------------------------------------------------------------//

#ifndef K_LOG_H
#define K_LOG_H

void msg(char *format, ...);

//----------------------------------------------------------------------------//
// implementation                                                             //
//----------------------------------------------------------------------------//

#ifdef K_LOG_IMPLEMENTATION

#include <stdio.h>
#include <stdarg.h>

//---- private functions -----------------------------------------------------//


//---- public functions ------------------------------------------------------//

void msg(char *format, ...) {
    va_list args;

	#ifdef K_LOG_PRINT_TO_SCREEN
	 	va_start(args, format);
		vprintf(format, args);
		fflush(stdout);
		va_end(args);
	#endif

	#ifdef K_LOG_PRINT_TO_FILE
		va_start(args, format);
		FILE *fp = fopen("log.txt", "w");
		vfprintf(fp, format, args);
		fclose(fp);
		va_end(args);
	#endif
}

#endif

#endif
