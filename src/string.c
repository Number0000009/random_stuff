#include <string.h>

void *memcpy(void *dest, const void *src, size_t n)
{
	char *d = dest;
	char *s = (char *)src;

	while(n--) {
		*d++ = *s++;
	}

	return dest;
}

void *memset(void *s, int c, size_t n)
{
	char *src = s;

	while(n--) {
		*src++ = c;
	}

	return s;
}
