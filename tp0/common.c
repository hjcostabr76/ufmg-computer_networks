#include "common.h"

#include <stdio.h>
#include <stdlib.h>

void commonLogErrorAndDie(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

void commonDebugStep(const char *text) {
	if (DEBUG_ENABLE) {
		printf("%s", text);
	}
}