/* *BSD doesn't have wchar.h or wcs functions */

#ifndef WCHAR_H
#define WCHAR_H

extern int wcslen(wchar_t *wc);
extern wchar_t *wcscpy(wchar_t *dst, const wchar_t *src);

#endif
