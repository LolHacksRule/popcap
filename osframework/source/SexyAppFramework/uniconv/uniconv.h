#ifndef __UNICONV_H__
#define __UNICONV_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * the conversion descriptor
     */
    struct _uniconv;
    typedef struct _uniconv uniconv_t;

#define UNICONV_SUCCESS  (0)
#define UNICONV_EILSEQ   (-1)
#define UNICONV_E2BIG    (-2)
#define UNICONV_EINVAL   (-3)
#define UNICONV_EBADF    (-4)

    /**
     * open a descriptor for converting /from/ charset to /to/ charset
     *
     * @param from the from charset
     * @param to the to charset
     *
     * @return a conversion descriptor, %NULL in case of error.
     */
    uniconv_t*
    uniconv_open(const char *to, const char *from);

    /**
     * peform charset converting
     *
     * If both inbuf and outbuf set to %NULL, then the descriptor
     * state will be reset. If only the inbuf set to %NULL, then the
     * descriptor will reset its shift state and write any pending
     * data to outbuf.
     *
     * @param inbuf a pointer to input buffer
     * @param inleft the size of input buffer
     * @param outbuf a pointer to output buffer
     * @param outleft the size of output buffer
     *
     * @return the number of characters converted in a non-reversible
     * way during this call.
     *
     * In case of error, it returns
     *
     *  UNICONV_E2BIG  There is not sufficient room at *outbuf.
     *  UNICONV_EILSEQ An invalid multibyte sequence has been encountered in the input.
     *  UNICONV_EINVAL An incomplete multibyte sequence has been encountered in the input.
     *
     */
    int
    uniconv_conv(uniconv_t *uc,
		 const char **inbuf,
		 size_t *inleft,
		 char **outbuf,
		 size_t *outleft);

    /**
     * close the conversion descriptor
     */
    void
    uniconv_close(uniconv_t *uc);

#ifdef __cplusplus
}
#endif

#endif

