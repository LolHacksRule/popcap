/*
 * multibytecodec.h: Common Multibyte Codec Implementation
 *
 * Written by Hye-Shik Chang <perky@FreeBSD.org>
 */

#ifndef _MULTIBYTECODEC_H_
#define _MULTIBYTECODEC_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#ifdef uint32_t
    typedef uint32_t ucs4_t;
#else
    typedef unsigned int ucs4_t;
#endif

#ifdef uint16_t
    typedef uint16_t ucs2_t, DBCHAR;
#else
    typedef unsigned short ucs2_t, DBCHAR;
#endif

    typedef union {
	void *p;
	int i;
	unsigned char c[8];
	ucs2_t u2[4];
	ucs4_t u4[2];
    } MultibyteCodec_State;

    typedef int (*mbcodec_init)(const void *config);
    typedef int (*mbencode_func)(MultibyteCodec_State *state,
				 const void *config,
				 const ucs4_t **inbuf, int inleft,
				 unsigned char **outbuf, int outleft,
				 int flags);
    typedef int (*mbencodeinit_func)(MultibyteCodec_State *state,
				     const void *config);
    typedef int (*mbencodereset_func)(MultibyteCodec_State *state,
				      const void *config,
				      unsigned char **outbuf, int outleft);
    typedef int (*mbdecode_func)(MultibyteCodec_State *state,
				 const void *config,
				 const unsigned char **inbuf, int inleft,
				 ucs4_t **outbuf, int outleft);
    typedef int (*mbdecodeinit_func)(MultibyteCodec_State *state,
				     const void *config);
    typedef int (*mbdecodereset_func)(MultibyteCodec_State *state,
				      const void *config);

    typedef struct {
	const char *encoding;
	const void *config;
	mbcodec_init codecinit;
	mbencode_func encode;
	mbencodeinit_func encinit;
	mbencodereset_func encreset;
	mbdecode_func decode;
	mbdecodeinit_func decinit;
	mbdecodereset_func decreset;
    } MultibyteCodec;

    typedef struct {
	const MultibyteCodec *codec;
	MultibyteCodec_State state;
    } MultibyteCodecState;

/* positive values for illegal sequences */
#define MBERR_TOOSMALL		(-1) /* insufficient output buffer space */
#define MBERR_TOOFEW		(-2) /* incomplete input buffer */
#define MBERR_INTERNAL		(-3) /* internal runtime error */

#define ERROR_STRICT		(1)
#define ERROR_IGNORE		(2)
#define ERROR_REPLACE		(3)

#define MBENC_FLUSH		0x0001 /* encode all characters encodable */
#define MBENC_MAX		MBENC_FLUSH

int
mbc_importmap(const char *modname, const char *symbol,
	      const void **encmap, const void **decmap);

int
mbcs_init(MultibyteCodecState *state,
	  const char *modname,
	  const char* charset);

void
mbcs_decode_init(MultibyteCodecState *state);

void
mbcs_decode_reset(MultibyteCodecState *state);

int
mbcs_decode(MultibyteCodecState *state,
	    const char** inbuf, size_t inlen,
	    ucs4_t** outbuf, size_t outlen);

#ifdef __cplusplus
}
#endif
#endif
