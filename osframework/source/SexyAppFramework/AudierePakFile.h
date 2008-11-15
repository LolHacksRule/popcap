#ifndef AUDIEREPAKFILE_H
#define AUDIEREPAKFILE_H

#include "audiere.h"
#include "../PakLib/PakInterface.h"

namespace Sexy
{
class AudierePakFile : public audiere::RefImplementation<audiere::File>
{
 private:
	AudierePakFile(PFILE * fp);

 public:
	static AudierePakFile* Open(const std::string& filename);
	~AudierePakFile();

	int  ADR_CALL read(void* buffer, int size);
	int  ADR_CALL write(const void* buffer, int size);
	bool ADR_CALL seek(int position, SeekMode mode);
	int  ADR_CALL tell();

 private:
	PFILE * mFp;
};

}

#endif
