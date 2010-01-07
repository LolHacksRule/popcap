#include "singlebytecodec.h"
#include "singlebytetables.h"
#include "uniconv.h"

#include <string.h>

int
sbcs_init(SingleByteCodecState *state, const char *encoding)
{
	SingleByteCodecState **codecs = __uniconv_get_single_byte_codecs();
	size_t i;

	if (!codecs)
		return -1;
	for (i = 0; codecs[i]; i++)
	{
		if (!strcmp(codecs[i]->encoding, encoding))
		{
			*state = *codecs[i];
			return 0;
		}
	}

	return -1;
}

static int
sbcs_encode_char(SingleByteCodecState *state,
		 uc_char_t unichar)
{
	size_t i;

	for (i = 0; i < state->encoding_map_size; i++)
		if (state->encoding_map[i].from == unichar)
			return state->encoding_map[i].to;
	return -1;
}

int
sbcs_encode(SingleByteCodecState *state,
	    const uc_char_t **inbuf,
	    size_t inleft,
	    char **outbuf,
	    size_t outleft)
{
	for (; inleft; inleft--, outleft--)
	{
		int result = sbcs_encode_char(state, **inbuf);
		if (result < 0)
			return UNICONV_EILSEQ;
		if (!outleft)
			return UNICONV_E2BIG;
		**outbuf = (char)result;
		(*outbuf)++;
		(*inbuf)++;
	}

	return 0;
}

int
sbcs_decode(SingleByteCodecState *state,
	    const char **inbuf,
	    size_t inleft,
	    uc_char_t **outbuf,
	    size_t outleft)
{
	const unsigned char **uinbuf = (const unsigned char **)inbuf;

	for (; inleft; inleft--, outleft--)
	{
		unsigned index = **uinbuf;

		if (index >= state->decoding_table_size)
			return UNICONV_EILSEQ;
		if (!outleft)
			return UNICONV_E2BIG;
		**outbuf = state->decoding_table[index];
		(*outbuf)++;
		(*uinbuf)++;
	}

	return 0;
}

