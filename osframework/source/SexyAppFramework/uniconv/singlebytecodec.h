#ifndef SINGLEBYTECODEC_H
#define SINGLEBYTECODEC_H

#include "uniconvint.h"

struct encoding_map {
    uc_char_t from;
    int to;
};

typedef struct _SingleByteCodecState {
    const char *encoding;
    const int *decoding_table;
    int decoding_table_size;
    const struct encoding_map *encoding_map;
    int encoding_map_size;
} SingleByteCodecState;

int
sbcs_init(SingleByteCodecState *state, const char *encoding);

int
sbcs_encode(SingleByteCodecState *state,
	    const uc_char_t **inbuf,
	    size_t inleft,
	    char **outbuf,
	    size_t outbytesleft);


int
sbcs_decode(SingleByteCodecState *state,
	    const char **inbuf,
	    size_t inbytesleft,
	    uc_char_t **outbuf,
	    size_t outleft);

#endif
