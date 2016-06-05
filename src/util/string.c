#include "string.h"

#include <string.h>

char *strAdd(char *buffer, int bufferLength, const char *str1, const char *str2) {
	bufferLength--;
	int lenStr1 = strlen(str1);
	int lenStr2 = strlen(str2);
	memcpy(buffer, str1, (lenStr1<bufferLength?lenStr1:bufferLength));
	int len2 = bufferLength - lenStr1;
	if (len2 > 0) memcpy(&buffer[lenStr1], str2, (lenStr2<len2?lenStr2:len2));
	int len = lenStr1 + lenStr2;
	buffer[(len<bufferLength?len:bufferLength)] = 0;
	return buffer;
}
