#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <assert.h>
#include <errno.h>

static char*
convert (const char *str,
	 size_t        len,
	 iconv_t       converter,
	 size_t       *bytes_read,
	 size_t       *bytes_written)
{
    char *dest;
    char *outp;
    const char *p;
    size_t inbytes_remaining;
    size_t outbytes_remaining;
    size_t err;
    size_t outbuf_size;
    int have_error = 0;
    int done = 0;
    int reset = 0;

    p = str;
    inbytes_remaining = len;
    outbuf_size = len + 1; /* + 1 for nul in case len == 1 */

    outbytes_remaining = outbuf_size - 1; /* -1 for nul */
    outp = dest = malloc (outbuf_size);

    while (!done && !have_error) {
	if (reset)
	    err = iconv (converter, NULL, &inbytes_remaining, &outp,
			 &outbytes_remaining);
	else
	    err = iconv (converter, (char **)&p, &inbytes_remaining,
			 &outp, &outbytes_remaining);

	if (err == (size_t) -1) {
	    switch (errno)
	    {
	    case EINVAL:
		/* Incomplete text, do not report an error */
		done = 1;
		break;
	    case E2BIG:
	    {
		size_t used = outp - dest;
		printf ("used: %zd, %zd(%zd)\n",
			used, inbytes_remaining, len);

		outbuf_size *= 2;
		dest = realloc (dest, outbuf_size);

		outp = dest + used;
		outbytes_remaining = outbuf_size - used - 1; /* -1 for nul */
	    }
	    break;
	    case EILSEQ:
		have_error = 1;
		break;
	    default:
		have_error = 1;
		break;
	    }
	} else {
	    if (!reset) {
		/* call iconv with NULL inbuf to cleanup shift state */
		reset = 1;
		inbytes_remaining = 0;
	    } else {
		done = 1;
	    }
	}
    }

    *outp = '\0';
    if (bytes_read) {
	*bytes_read = p - str;
    } else {
	if ((p - str) != len) {
	    if (!have_error)
		have_error = 1;
	}
    }

    if (bytes_written)
	*bytes_written = outp - dest;	/* Doesn't include '\0' */

    if (have_error) {
	free (dest);
	return NULL;
    }

    return dest;
}

#define UCONV_BUFSZ 4096
int main(int argc, char **argv)
{
    FILE *infp, *outfp;
    const char *from, *to;
    iconv_t conv;
    char *inbuf;
    char *outbuf;
    size_t inlen;
    size_t outlen;

    if (argc < 5)
	return -1;

    from = argv[1];
    to = argv[2];

    conv = iconv_open(to, from);
    if (conv == (iconv_t)-1) {
	fprintf (stderr,
		 "Converting from %s to %s is unsupported.\n",
		 from, to);
	return -1;
    }

    infp = fopen(argv[3], "rb");
    if (!infp)
	return -1;

    fseek(infp, 0, SEEK_END);
    inlen = ftell(infp);
    fseek(infp, 0, SEEK_SET);
    inbuf = malloc(sizeof(char) * (inlen + 1));
    if (!inbuf)
	return -1;

    fread(inbuf, 1, inlen, infp);
    outbuf = convert(inbuf, inlen, conv, NULL, &outlen);
    if (!outbuf) {
	fprintf(stderr, "Failed to convert.\n");
    }

    outfp = fopen(argv[4], "wb");
    if (!outfp)
	return -1;

    if (outbuf)
	fwrite(outbuf, 1, outlen, outfp);

    free(inbuf);
    free(outbuf);

    fclose(outfp);
    fclose(infp);

    iconv_close(conv);

    return 0;
}
