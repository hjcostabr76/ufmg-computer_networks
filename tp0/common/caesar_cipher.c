#include "common.h"

const int RANGE = ASCII_LC_LETTER_LAST - ASCII_LC_LETTER_FIRST;

/**
 * FIXME: 2021-06-07 - Algo errado nao esta certo...
 */
void caesarCipher(const char *text, const int textLength, char *cipheredText, int key) {

	for (int i = 0; i < textLength; i++) {

		const int currentCharCode = (int)text[i];
		int cipheredCharCode = currentCharCode + key;
		
		while (cipheredCharCode > ASCII_LC_LETTER_LAST) {
			cipheredCharCode -= (RANGE + 1);
		}
		
		cipheredText[i] = (char)cipheredCharCode;
	}

	cipheredText[textLength] = '\0';
}

/**
 * FIXME: 2021-06-07 - Algo errado nao esta certo...
 */
void caesarDecipher(const char *cipheredText, const int textLength, char *text, int key) {

	for (int i = 0; i < textLength; i++) {

		const int cipheredCharCode = (int)text[i];
		int charCode = cipheredCharCode - key;
		
		while (charCode < ASCII_LC_LETTER_FIRST) {
			charCode += (RANGE + 1);
		}
		
		text[i] = (char)charCode;
	}

	text[textLength] = '\0';
}