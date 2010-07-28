#ifndef __SEXY_PAKLIB_FIND_H__
#define __SEXY_PAKLIB_FIND_H__

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#ifndef WIN32
#include <unistd.h>
#else
#include <io.h>
#endif

namespace PakLib {

#ifndef WIN32

/* Our simplified data entry structure */
struct _finddata_t
{
    unsigned attrib;
    time_t          time_create;
    time_t          time_access;    /* always midnight local time */
    time_t          time_write;
    char *name;
    unsigned long size;
};

#define _A_NORMAL 0x00  /* Normalfile-Noread/writerestrictions */
#define _A_RDONLY 0x01  /* Read only file */
#define _A_HIDDEN 0x02  /* Hidden file */
#define _A_SYSTEM 0x04  /* System file */
#define _A_ARCH   0x20  /* Archive file */
#define _A_SUBDIR 0x10  /* Subdirectory */

intptr_t _findfirst(const char *pattern, struct _finddata_t *data);
int _findnext(intptr_t id, struct _finddata_t *data);
int _findclose(intptr_t id);

#endif

};

#endif
