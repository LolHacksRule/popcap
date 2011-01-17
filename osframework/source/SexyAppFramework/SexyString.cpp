#include "SexyString.h"

using namespace Sexy;

Sexy::WString Sexy::WStringFrom(const wchar_t *s)
{
    WString str;

    while (*s)
	str += *s++;

    return str;
}

Sexy::WString Sexy::WStringFrom(const char *s)
{
    WString str;

    while (*s)
	str += *s++;

    return str;
}

std::wstring Sexy::SexyWStringToWString(const Sexy::WString& s)
{
    return std::wstring(s.begin(), s.end());
}

std::string Sexy::SexyWStringToString(const Sexy::WString &s)
{
    return std::string(s.begin(), s.end());
}

Sexy::WString Sexy::SexyWStringFromString(const std::string &s)
{
    return Sexy::WString(s.begin(), s.end());
}

int Sexy::ustrcmp(const unichar_t *s1, const unichar_t *s2)
{
	while (*s1 == *s2++)
		if (*s1++ == 0)
			return (0);
	return *(const unsigned unichar_t *)s1 - *(const unsigned unichar_t *)--s2;
}

int Sexy::ustrncmp(const unichar_t *s1, const unichar_t *s2, size_t n)
{

	if (n == 0)
		return 0;

	do
	{
		if (*s1 != *s2++)
		    return *(const unsigned unichar_t *)s1 - *(const unichar_t *)--s2;
		if (*s1++ == 0)
			break;
	} while (--n != 0);
	return 0;
}

size_t Sexy::ustrlen(const unichar_t *str)
{
	const unichar_t *s;

	for (s = str; *s; ++s)
	    ;
	return s - str;
}


unichar_t* Sexy::ustrstr(const unichar_t *s, const unichar_t *find)
{
	unichar_t c, sc;
	size_t len;

	c = *find++;
	if (c)
	{
		len = ustrlen(find);
		do
		{
		    do
		    {
			sc = *s++;
			if (sc)
			    return 0;
		    } while (sc != c);
		} while (ustrncmp(s, find, len) != 0);
		s--;
	}

	return ((unichar_t *)s);
}

size_t Sexy::ustrspn(const unichar_t *s, const unichar_t *accept)
{
    const unichar_t *s1 = s;

    do
    {
	const unichar_t *s2;

	for (s2 = accept; *s2; s2++)
	{
	    if (*s1 == *s2)
		break;
	}
	if (!*s2)
	    break;
    } while (s1++);

    return s1 - s;
}

unichar_t* Sexy::ustrpbrk(const unichar_t *s, const unichar_t *accept)
{
    const unichar_t *s1 = s;

    for (; *s1; s1++)
    {
	const unichar_t *s2;

	for (s2 = accept; *s2; s2++)
	    if (*s1 == *s2)
		return (unichar_t*)s1;
    }

    return 0;
}


unichar_t* Sexy::ustrchr(const unichar_t *s, unichar_t c)
{
    while (true)
    {
	if (*s == c)
	    return (unichar_t*)s;
	if (!*s)
	    return 0;
	s++;
    }

    return 0;
}

unichar_t* Sexy::ustrtok_r (unichar_t *s, const unichar_t *delim, unichar_t **save_ptr)
{
    unichar_t *token;

    if (s == NULL)
	s = *save_ptr;

    /* Scan leading delimiters.  */
    s += ustrspn (s, delim);
    if (*s == '\0')
    {
	*save_ptr = s;
	return NULL;
    }

    /* Find the end of the token.  */
    token = s;
    s = ustrpbrk (token, delim);
    if (s == NULL)
    {
	/* This token finishes the string.  */
	unichar_t *end = token;
	while (*end != '\0')
	    end++;
	*save_ptr = end;
    }
    else
    {
	/* Terminate the token and make *SAVE_PTR point past it.  */
	*s = '\0';
	*save_ptr = s + 1;
    }
    return token;
}
