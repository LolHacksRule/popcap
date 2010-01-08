#ifndef CONVERTER_H
#define CONVERTER_H

#include <uniconvint.h>

struct converter {
    int
    (*encode)(struct converter *conv,
	      const uc_char_t **inbuf,
	      size_t inleft,
	      char **outbuf,
	      size_t outbytesleft);

    int
    (*decode)(struct converter *conv,
	      const char **inbuf,
	      size_t inbytesleft,
	      uc_char_t **outbuf,
	      size_t outleft);

    void
    (*reset)(struct converter *conv);

    void
    (*close)(struct converter *conv);
};

struct converter*
converter_open(const char *charset);

void
converter_close(struct converter *converter);

void
converter_reset(struct converter *converter);

#endif
