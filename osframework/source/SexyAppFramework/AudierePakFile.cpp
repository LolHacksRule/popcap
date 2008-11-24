#include "Common.h"
#include "AudierePakFile.h"

#include <string>

using namespace audiere;
using namespace Sexy;

AudierePakFile* AudierePakFile::Open(const std::string& filename)
{
	PFILE * fp;

	fp = p_fopen (filename.c_str (), "rb");
	if (!fp)
		return 0;

	return new AudierePakFile(fp);
}

AudierePakFile::AudierePakFile(PFILE * fp)
{
	mFp = fp;
}

AudierePakFile::~AudierePakFile()
{
	p_fclose (mFp);
}

int ADR_CALL AudierePakFile::read(void* buffer, int size)
{
	return p_fread (buffer, 1, size, mFp);
}

int ADR_CALL AudierePakFile::write(const void* buffer, int size)
{
	return -1;
}

bool ADR_CALL AudierePakFile::seek(int position, SeekMode mode)
{
	int m;
	switch (mode) {
        case BEGIN:
		m = SEEK_SET;
		break;
        case CURRENT:
		m = SEEK_CUR;
		break;
        case END:
		m = SEEK_END;
		break;
        default:
		return false;
	}

	return p_fseek (mFp, position, m) == 0;
}

int ADR_CALL AudierePakFile::tell()
{
	return p_ftell (mFp);
}
