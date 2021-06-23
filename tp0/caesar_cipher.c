#include "common.h"

const int CHAR_CODE_RANGE = ASCII_LC_LETTER_LAST - ASCII_LC_LETTER_FIRST;

void caesarCipher(const char *text, const int textLength, char *cipheredText, int key) {

	for (int i = 0; i < textLength; i++) {

		const int currentCharCode = (int)text[i];
		int cipheredCharCode = currentCharCode + key;
		
		while (cipheredCharCode > ASCII_LC_LETTER_LAST) {
			cipheredCharCode -= (CHAR_CODE_RANGE + 1);
		}
		
		cipheredText[i] = (char)cipheredCharCode;
	}

	cipheredText[textLength] = '\0';
}

void caesarDecipher(const char *cipheredText, const int textLength, char *text, int key) {


	for (int i = 0; i < textLength; i++) {

		const int cipheredCharCode = (int)cipheredText[i];
		int charCode = cipheredCharCode - key;
		
		while (charCode < ASCII_LC_LETTER_FIRST) {
			charCode += (CHAR_CODE_RANGE + 1);
		}
		
		text[i] = (char)charCode;
	}

	text[textLength] = '\0';
}