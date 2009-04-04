#ifndef __FREETYPEBASEFONT_H__
#define __FREETYPEBASEFONT_H__

#include "AutoCrit.h"
#include "PakInterface.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace Sexy
{

	class FreeTypeBaseFont
	{
	public:
		FreeTypeBaseFont(PFILE * fp, int index);
	private:
		~FreeTypeBaseFont();

	public:
		void               Ref();
		void               Unref();

		FT_Face            LockFace(float size, FT_Matrix* matrix);
		void               SetSize(float size, FT_Matrix* matrix);
		void               UnlockFace();
		bool               IsFaceLocked();
		bool               HasUnlockedFace();
		bool               DropUnlockedFace();

	 private:
		int                mRefCnt;
		int                mLockCnt;
		PFILE*             mFp;
		int                mIndex;
		bool               mHasSize;
		float              mSize;
		FT_Matrix          mMatrix;
		FT_Face            mFace;

		FT_Stream          mStream;

		CritSect           mCritSect;
		CritSect           mRefCritSect;
	};

}

#endif
