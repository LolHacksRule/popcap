#ifndef __SEXY_STRING_H__
#define __SEXY_STRING_H__

#include <string>

namespace Sexy {

	typedef std::string String;

#if defined(WIN32) || defined(_WIN32)
	typedef short unichar_t;
#else
	typedef int unichar_t;
#endif
	typedef std::basic_string<unichar_t> WString;

	WString WStringFrom(const wchar_t *s);
	WString WStringFrom(const char *s);

	std::string SexyWStringToString(const Sexy::WString &s);
	Sexy::WString SexyWStringFromString(const std::string &s);

	inline WString WSTR(const wchar_t *s)
	{
		return WStringFrom(s);
	}

	inline WString WSTR(const char *s)
	{
		return WStringFrom(s);
	}

	inline WString operator+(const WString& lhs, const wchar_t *rhs)
	{
		WString str(lhs);

		while (*rhs)
			str.push_back(*rhs++);
		return str;
	}

	inline WString operator+(const wchar_t *lhs, const WString& rhs)
	{
		WString str;

		while (*lhs)
			str.push_back(*lhs++);
		return str + rhs;
	}

	inline bool operator==(const WString& lhs, const wchar_t *rhs)
	{
		WString str = WSTR(rhs);
		return lhs == str;
	}

	inline bool operator==(const wchar_t *lhs, const WString& rhs)
	{
		WString str = WSTR(lhs);
		return rhs == str;
	}

	inline bool operator!=(const WString& lhs, const wchar_t *rhs)
	{
		WString str = WSTR(rhs);
		return lhs != str;
	}

	inline bool operator!=(const wchar_t *lhs, const WString& rhs)
	{
		WString str = WSTR(lhs);
		return rhs != str;
	}

	int ustrcmp(const unichar_t *s1, const unichar_t *s2);
	int ustrncmp(const unichar_t *s1, const unichar_t *s2, size_t n);
	size_t ustrlen(const unichar_t *str);
	unichar_t* ustrstr(const unichar_t *s, const unichar_t *find);
	size_t ustrspn(const unichar_t *s, const unichar_t *accept);
	unichar_t* ustrpbrk(const unichar_t *s, const unichar_t *accept);
	unichar_t* ustrchr(const unichar_t *s, unichar_t c);
	unichar_t* ustrtok_r(unichar_t *s, const unichar_t *delim, unichar_t **save_ptr);
}

#endif
