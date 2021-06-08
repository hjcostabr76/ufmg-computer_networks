
#define ASCII_NUMBER_FIRST 48
#define ASCII_NUMBER_LAST 57
#define ASCII_LC_LETTER_LAST 122
#define ASCII_LC_LETTER_FIRST 97

int stringValidateLCaseString(const char *string, const int strLength) {

	for (int i; i < strLength; i++) {
		if ((int)string[i] < ASCII_LC_LETTER_FIRST || (int)string[i] > ASCII_LC_LETTER_LAST)
			return 0;
	}

	return 1;
}

int stringValidateNumericString(const char *string, const int strLength) {

	for (int i; i < strLength; i++) {
		if ((int)string[i] < ASCII_NUMBER_FIRST || (int)string[i] > ASCII_NUMBER_LAST)
			return 0;
	}

	return 1;
}