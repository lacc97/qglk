/* *BSD does not have wcs functions. */
#if defined(FREEBSD_386) || defined(OPENBSD) || defined(DARWIN)

#include <stdlib.h>

int wcslen(wchar_t *wc)
{
	int len = 0;
	while (*wc++)
		++len;
	return len;
}

wchar_t *wcscpy(wchar_t *dst, const wchar_t *src)
{
	wchar_t *start = dst;
	while ((*dst++ = *src++) != 0)
		;
	return start;
}

#endif /* FREEBSD_386 || OPENBSD || DARWIN */
