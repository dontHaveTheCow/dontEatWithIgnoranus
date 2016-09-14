#ifndef STRING_LIBRARY
#define STRING_LIBRARY

#include "string.h"
#include <stdlib.h>
#include "assert.h"

void reverse(char s[]);
void itoa(int n, char s[]);
void str_splitter(char* buffer, char* _key, char* _value, char* delimiter);

#endif
