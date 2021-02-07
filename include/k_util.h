//============================================================================//
// k_util.h                                                                //
//============================================================================//

//----------------------------------------------------------------------------//
// interface                                                                  //
//----------------------------------------------------------------------------//

#ifndef K_UTIL_H
#define K_UTIL_H

// #define K_LOG_FILE "log.txt"
// #define K_LOG_PRINT_TO_SCREEN
// #define K_LOG_PRINT_TO_FILE

void msg(char *format, ...);

//----------------------------------------------------------------------------//
// implementation                                                             //
//----------------------------------------------------------------------------//

#ifdef K_UTIL_IMPLEMENTATION

#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>

//---- private functions -----------------------------------------------------//


//---- public functions ------------------------------------------------------//

double get_time() {
	struct timeval time;
	gettimeofday(&time, NULL);

	return (double)time.tv_sec + (double)time.tv_usec / 1000000;
}

int sec_to_min(int seconds) {
	return seconds / 60;
}

int sec_to_h(int seconds) {
	return sec_to_min(seconds) / 60;
}

/*
char *read_file(char *fileName) {
	FILE *fp;
	char *buffer;
 
	fp = fopen(fileName, "rb");
	buffer = malloc(0x100000);
	fread(buffer, 1, 0x100000, fp);
	fclose(fp);

	return buffer;
}
*/

char *read_file(char *fileName) {
	char *source = NULL;
	FILE *fp = fopen(fileName, "r");

	if (fp != NULL) {
		if (fseek(fp, 0L, SEEK_END) == 0) {
			long bufsize = ftell(fp);
			source = malloc(sizeof(char) * (bufsize + 1));
			fseek(fp, 0L, SEEK_SET);

			size_t newLen = fread(source, sizeof(char), bufsize, fp);

			if (ferror(fp) != 0)
				return NULL;
			else
				source[newLen++] = '\0';
		}

		fclose(fp);
	}

	return source;
}


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
		FILE *fp = fopen(K_LOG_FILE, "a");
		vfprintf(fp, format, args);
		fclose(fp);
		va_end(args);
	#endif
}

#endif

#endif
