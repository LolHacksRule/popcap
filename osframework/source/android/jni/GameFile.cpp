#include "GameFile.h"
#include "GameLauncher.h"
#include "ACPManager.h"

#include <stdlib.h>
#include <errno.h>

#include <android/log.h>

#define  LOG_TAG    "libGameLauncher/File"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern "C" {

        struct _awFile {
                char*         mFilename;
                long          mPos;
                unsigned long mSize;
                char*         mBytes;
                int           mError;
        };

        awFile* awFileOpen(const char *filename)
        {
                GameLauncher* launcher = GameLauncher::getInstance();

                LOGI("try to open %s\n", filename);
                if (!launcher->getJNIEnv())
                        return 0;

                if (!filename)
                {
                        errno = EINVAL;
                        return 0;
                }

                if (!awHasFile(launcher->getJNIEnv(), filename))
                {
                        errno = ENOENT;
                        return 0;
                }

                awFile *file = (awFile*)malloc(sizeof(awFile) + strlen(filename) + 1);
                if (!file)
                        return 0;

                file->mFilename = (char*)(file + 1);
                memcpy(file->mFilename, filename, strlen(filename) + 1);
                file->mPos = 0;
                file->mSize = awGetFileSize(launcher->getJNIEnv(), filename);
                file->mBytes = 0;
                file->mError = 0;

                LOGI("%s size: %lu\n", filename, file->mSize);

                return file;
        }

        static int awFileLoad(awFile *fp)
        {
                GameLauncher* launcher = GameLauncher::getInstance();

                if (fp->mBytes)
                        return 0;

                if (!launcher->getJNIEnv())
                        return 0;

                int size;
                if (awGetFile(launcher->getJNIEnv(), fp->mFilename,
                              (void**)&fp->mBytes, &size) == 0)
                        return 0;
                return -1;
        }

        long awFileSeek(awFile *fp, long offset, int whence)
        {
                long pos;

                switch (whence)
                {
                case SEEK_SET:
                        pos = offset;
                        break;
                case SEEK_CUR:
                        pos = fp->mPos + offset;
                        break;
                case SEEK_END:
                        pos = fp->mSize - offset;
                        break;
                }

                if (pos < 0 || pos > fp->mSize)
                {
                        errno = EINVAL;
                        return -1;
                }
                fp->mPos = pos;
                return 0;
        }

        size_t awFileRead(awFile *fp, void *ptr, size_t size, size_t nmemb)
        {
                if (!fp || !ptr || !size || !nmemb || fp->mError)
                        return 0;

                if (!fp->mBytes)
                        awFileLoad(fp);
                if (!fp->mBytes)
                {
                        fp->mError = 1;
                        return 0;
                }

                //LOGI("try to read %zd(%zd) bytes from %s[%lu:%lu]\n",
                //     size, nmemb, fp->mFilename, fp->mPos, fp->mSize);

                if (!fp->mSize)
                        return 0;

                size_t left = fp->mSize - fp->mPos;
                if (!left)
                        return 0;

                size_t left_n = left / size;
                if (nmemb > left_n)
                        nmemb = left_n;
                memcpy(ptr, fp->mBytes + fp->mPos, size * nmemb);
                fp->mPos += size * nmemb;

                //LOGI("try to read %zd(%zd) bytes from %s[%lu:%lu]\n",
                //     size, nmemb, fp->mFilename, fp->mPos, fp->mSize);

                return nmemb;
        }

        long awFileTell(awFile *fp)
        {
                if (!fp)
                {
                        errno = EBADF;
                        return -1;
                }
                return fp->mPos;
        }

        int awFileEof(awFile *fp)
        {
                if (!fp)
                {
                        errno = EBADF;
                        return -1;
                }

                if (!fp->mSize)
                        return 1;

                return fp->mPos == fp->mSize - 1;
        }

        int awFileError(awFile *fp)
        {
                if (!fp)
                {
                        errno = EBADF;
                        return -1;
                }

                return fp->mError != 0;
        }

        void awFileClose(awFile *fp)
        {
                if (!fp)
                        return;

                LOGI("closing file %s.\n", fp->mFilename);

                free(fp->mBytes);
                free(fp);
        }

}
