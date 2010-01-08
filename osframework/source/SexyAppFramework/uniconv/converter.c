#include "converter.h"
#include "utfconverter.h"
#include "tableconverter.h"

typedef struct converter* (*converterfunc)(const char *charset);

struct converter*
converter_open(const char *charset)
{
    static converterfunc funcs[] = {
	tabconverter_open,
	utfconverter_open,
	NULL
    };
    size_t i;
    struct converter *conv;

    for (i = 0; funcs[i]; i++) {
	conv = funcs[i](charset);
	if (conv)
	    return conv;
    }

    return NULL;
}

void
converter_close(struct converter *conv)
{
    if (!conv)
	return;

    conv->close(conv);
}

void converter_reset(struct converter *conv)
{
    if (conv->reset)
	conv->reset(conv);
}
