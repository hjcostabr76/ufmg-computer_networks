#include "common.h"

#include <stdlib.h>

void commonLogErrorAndDie(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

void commonDebugStep(const char *text) {
	if (DEBUG_ENABLE) {
		puts(text);
	}
}