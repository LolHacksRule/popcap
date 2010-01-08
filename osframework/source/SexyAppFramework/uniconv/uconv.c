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

    conv = uniconv_open(to, from);
    if (!conv) {
	fprintf (stderr,
		 "Converting from %s to %s is unsupported.\n",
		 from, to);
	return -1;
    }

    infp = fopen(argv[3], "rb");
    if (!infp)
	return -1;
    outfp = fopen(argv[4], "wb");
    if (!outfp)
	return -1;

    off = 0;
    while (!feof(infp)) {
	int ret;
	char *inp;
	char *outp;
	size_t len;

	len = inlen = fread(inbuffer, 1, UCONV_BUFSZ, infp);
	while (1) {
	    size_t inleft = inlen;
	    size_t outleft = sizeof(outbuffer);

	    inp = inbuffer;
	    outp = outbuffer;
	    ret = uniconv_conv(conv, (const char **)&inp, &inleft, &outp, &outleft);
	    if (ret == UNICONV_EINVAL && inlen < sizeof(inbuffer)) {
		size_t extralen = fread(inbuffer + inlen, 1, 1, infp);
		if (!extralen)
		    break;
		len += extralen;
		inlen += extralen;
		continue;
	    }
	    if (ret) {
		fprintf (stderr, "Failed to convert: %d.\n", ret);
		exit(-1);
		break;
	    }
	    assert (inp - inbuffer == inlen);
	    fwrite(outbuffer, 1, outp - outbuffer, outfp);
	    break;
	}
	off += len;
    }

    while (1) {
	char *outp;
	int ret;
	size_t outleft;

	outp = outbuffer;
	outleft = sizeof(outbuffer);
	ret = uniconv_conv(conv, NULL, NULL, &outp, &outleft);
	if (ret < 0) {
	    fprintf (stderr, "Failed to flush descriptor.\n");
	    break;
	} else {
	    fwrite(outbuffer, 1, outp - outbuffer, outfp);
	    break;
	}
    }

    fclose(outfp);
    fclose(infp);

    uniconv_close(conv);

    return 0;
}
