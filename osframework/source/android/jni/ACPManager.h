#ifndef __ACP_MANAGER_H__
#define __ACP_MANAGER_H__

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

// Returns 1 if the file exists in the assets
unsigned char awHasFile(JNIEnv *env, const char *name);

unsigned long awGetFileSize(JNIEnv *env, const char *name);

// Allocates and fills the data with the package of the given name
// Returns 0 for success
int awGetFile(JNIEnv *env, const char *name, void **ptr, int *size);

#ifdef __cplusplus
}
#endif

#endif
