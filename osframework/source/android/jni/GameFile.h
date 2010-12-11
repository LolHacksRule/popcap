#ifndef __GAME_FILE_H__
#define __GAME_FILE_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _awFile awFile;

awFile* awFileOpen(const char *filename);

long awFileSeek(awFile *fp, long offset, int whence);

size_t awFileRead(awFile *fp, void *ptr, size_t size, size_t nmemb);

long awFileTell(awFile *fp);

int awFileEof(awFile *fp);

int awFileError(awFile *fp);

void awFileClose(awFile *fp);

#ifdef __cplusplus
}
#endif

#endif
