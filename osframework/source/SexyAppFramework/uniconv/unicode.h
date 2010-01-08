#ifndef UNICODE_H
#define UNICODE_H

#include <uniconvint.h>

int
ucs4toutf16(uc_char_t unichar, uc_uint16_t *utf16);

int
ucs4fromutf16(const uc_uint16_t *utf16, uc_char_t *ucs4, int len);

int
ucs4toutf8(uc_char_t unichar, char *utf8);

int
ucs4fromutf8(const char *utf8, uc_char_t * ucs4, int len);

#endif
