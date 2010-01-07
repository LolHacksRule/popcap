#include <stdio.h>
#include <stdlib.h>
#include <uniconv.h>
#include <assert.h>

#define UCONV_BUFSZ 4096
int main(int argc, char **argv)
{
    FILE *infp, *outfp;
    const char *from, *to;
    char inbuffer[UCONV_BUFSZ + 8];
    char outbuffer[UCONV_BUFSZ * 6];
    size_t inlen;
    uniconv_t *conv;
    size_t off;

    if (argc < 5)
	return -1;

    from = argv[1];
    to = argv[2];
    infp = fopen(argv[3], "rb");
    if (!infp)
	return -1;
    outfp = fopen(argv[4], "wb");
    if (!outfp)
	return -1;

    conv = uniconv_open(from, to);
    if (!conv)
	return -1;

    off = 0;
    while (!feof(infp)) {
	int ret;
	char *inp;
	char *outp;
	size_t len;

	len = inlen = fread(inbuffer, 1, UCONV_BUFSZ, infp);
	while (1) {
	    inp = inbuffer;
	    outp = outbuffer;
	    ret = uniconv_conv(conv, (const char **)&inp, inlen, &outp, sizeof(outbuffer));
	    if (ret == UNICONV_E2BIG && inlen < sizeof(inbuffer)) {
		size_t extralen = fread(inbuffer + inlen, 1, 1, infp);
		if (!extralen)
		    break;
		len += extralen;
		inlen += extralen;
		continue;
	    }
	    if (ret) {
		printf ("Failed to convert.\n");
		exit(-1);
		break;
	    }
	    assert (inp - inbuffer == inlen);
	    fwrite(outbuffer, 1, outp - outbuffer, outfp);
	    break;
	}
	off += len;
    }

    fclose(outfp);
    fclose(infp);

    uniconv_close(conv);

    return 0;
}
