#include "common.h"

#include <stdio.h>
#include <stdlib.h>

void comLogErrorAndDie(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

void comDebugStep(const char *text) {
	if (DEBUG_ENABLE)
		printf("%s", text);
}

int comValidateLCaseString(const char *string, const int strLength) {

	for (int i; i < strLength; i++) {
		if ((int)string[i] < ASCII_LC_LETTER_FIRST || (int)string[i] > ASCII_LC_LETTER_LAST)
			return 0;
	}

	return 1;
}

int comValidateNumericString(const char *string, const int strLength) {

	for (int i; i < strLength; i++) {
		if ((int)string[i] < ASCII_NUMBER_FIRST || (int)string[i] > ASCII_NUMBER_LAST)
			return 0;
	}

	return 1;
}