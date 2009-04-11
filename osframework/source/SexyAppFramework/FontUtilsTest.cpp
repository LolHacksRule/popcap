#include "FontUtils.h"
#include "assert.h"
#include <string>

using namespace Sexy;

static void test (std::string &source,
		  std::string &expected,
		  size_t num_ucs4)
{
    std::string result;
    int ret;

    ret = SexyUtf8Strlen (expected.c_str(), -1);
    assert (ret == (int)num_ucs4);

    ret = SexyUtf8FromString (source, result);
    assert (ret == (int)num_ucs4);
    assert (result == expected);
}

int main (int argc, char **argv)
{
    struct tests {
	std::string source;
	std::string expected;
	size_t num_ucs4;
    } tests[] = {
	{ "\xc4\xe3\xba\xc3", "\xe4\xbd\xa0\xe5\xa5\xbd", 2 },
	{ "\xc1\xb7\xcf\xb0\xc4\xa3\xca\xbd", "\xe7\xbb\x83\xe4\xb9\xa0\xe6\xa8\xa1\xe5\xbc\x8f", 4 },
	{ "", "", 0 }
    };

    for (size_t i = 0; tests[i].source.length(); i++)
	test(tests[i].source, tests[i].expected, tests[i].num_ucs4);

    return 0;
}
