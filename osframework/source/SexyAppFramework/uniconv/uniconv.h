#ifndef __UNICONV_H__
#define __UNICONV_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

    struct _uniconv;
    typedef struct _uniconv uniconv_t;

#define UNICONV_SUCCESS  (0)
#define UNICONV_EILSEQ   (-1)
#define UNICONV_E2BIG    (-2)
#define UNICONV_EINVAL   (-3)
#define UNICONV_EBADF     (-4)

    uniconv_t*
    uniconv_open(const char *from, const char *to);

    int
    uniconv_conv(uniconv_t *uc,
		 const char **inbuf,
		 size_t inleft,
		 char **outbuf,
		 size_t outleft);

    void
    uniconv_close(uniconv_t *uc);

#ifdef __cplusplus
}
#endif

#endif

