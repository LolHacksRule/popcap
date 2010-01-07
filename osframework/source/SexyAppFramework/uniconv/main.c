#include <assert.h>
#include <string.h>

#include <multibytecodec.h>

int main()
{
    MultibyteCodecState state;
    char hello[] = "\xc4\xe3\xba\xc3";
    ucs4_t expected[] = { 0x4f60, 0x597d, 0x0};
    ucs4_t result[3];
    const char* inp;
    ucs4_t *outp;
    int ret;

    ret = mbcs_init(&state, "cn", "gbk");
    assert (ret == 0);

    inp = hello;
    outp = result;

    ret = mbcs_decode(&state, &inp, 3, &outp, 3);
    assert (ret == MBERR_TOOFEW);

    inp = hello;
    outp = result;
    ret = mbcs_decode(&state, &inp, 4, &outp, 1);
    assert (ret == MBERR_TOOSMALL);

    inp = hello;
    outp = result;
    ret = mbcs_decode(&state, &inp, 4, &outp, 3);
    assert (ret == 0);

    assert(!memcmp(result, expected, sizeof(ucs4_t) * 2));

    return 0;
}
