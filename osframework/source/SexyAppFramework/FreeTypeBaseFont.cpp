#include "FreeTypeBaseFont.h"
#include "FreeTypeFontMap.h"

#include <assert.h>
#include <math.h>

using namespace Sexy;

#define HAVE_FT_BITMAP_SIZE_Y_PPEM 1

static unsigned long
FreeTypePakRead (FT_Stream stream,
		 unsigned long offset,
		 unsigned char * buffer,
		 unsigned long count)
{
    PFILE * fp = (PFILE*)stream->descriptor.pointer;

    assert (fp != NULL);
    if (offset > stream->size)
	return 0;

    if (offset + count > stream->size)
	count = stream->size - offset;

    p_fseek(fp, offset, SEEK_SET);
    if (p_fread(buffer, 1, count, fp) != count)
	    return 0;
    return count;
}

FreeTypeBaseFont::FreeTypeBaseFont(PFILE * fp, int index) :
	mRefCnt(1), mLockCnt(0), mFp(fp), mIndex(index), mHasSize(false),
	mSize(0), mFace(0), mStream(0)
{
}

FreeTypeBaseFont::~FreeTypeBaseFont()
{
	FreeTypeFontMap* aFontMap = FreeTypeFontMap::GetFreeTypeFontMap();
	aFontMap->RemoveBaseFont(this);
	DropUnlockedFace();
	p_fclose(mFp);
	if (mStream)
		delete mStream;
}

void FreeTypeBaseFont::Ref()
{
	AutoCrit anAutoCrit(mRefCritSect);

	mRefCnt++;
}

void FreeTypeBaseFont::Unref()
{
	{
		AutoCrit anAutoCrit(mRefCritSect);

		if (--mRefCnt)
			return;
	}

	delete this;
}

FT_Face	 FreeTypeBaseFont::LockFace(float size, FT_Matrix* matrix)
{
	FreeTypeFontMap* aFontMap = FreeTypeFontMap::GetFreeTypeFontMap();

	mCritSect.Enter();
	mLockCnt++;

	if (mFace)
	{
		SetSize(size, matrix);
		return mFace;
	}

	aFontMap->ReserveFace(this);

	FT_Open_Args args;

	if (!mStream)
		mStream = new FT_StreamRec_;

	p_fseek(mFp, 0, SEEK_END);
	memset (mStream, 0, sizeof (*mStream));
	mStream->descriptor.pointer = (void*)mFp;
	mStream->read = FreeTypePakRead;
	mStream->pos = 0;
	mStream->size = p_ftell(mFp);

	args.flags = FT_OPEN_STREAM;
	args.stream = mStream;
	if (FT_Open_Face(aFontMap->mLibrary, &args,
			 mIndex, &mFace) != FT_Err_Ok)
	{
		mLockCnt--;
		mCritSect.Leave();
		aFontMap->ReleaseFace(this);
		return 0;
	}

	SetSize(size, matrix);
	return mFace;
}

void  FreeTypeBaseFont::SetSize(float size, FT_Matrix* matrix)
{
    FT_Error error;
    FT_Face face;
    FT_Matrix ftmatrix;

    if (mHasSize && mSize == size && !memcmp (&mMatrix, matrix, sizeof(*matrix)))
	return;

    face = mFace;
    assert (face != NULL);

    mHasSize = true;
    mSize = size;
    mMatrix = *matrix;

    ftmatrix.xx = matrix->xx;
    ftmatrix.xy = -matrix->xy;
    ftmatrix.yx = -matrix->yx;
    ftmatrix.yy = matrix->yy;

    FT_Set_Transform(face, &ftmatrix, NULL);

    if (FT_IS_SCALABLE(face))
    {
	    error = FT_Set_Char_Size (face,
				      size * 64.0,
				      size * 64.0,
				      0, 0);
    }
    else
    {
	    double min_distance = -1;
	    int i;
	    int best = 0;

	    for (i = 0; i < face->num_fixed_sizes; i++)
	    {
#if HAVE_FT_BITMAP_SIZE_Y_PPEM
		    double fsize = face->available_sizes[i].y_ppem / 64.;
#else
		    double fsize = face->available_sizes[i].height;
#endif
		    double distance = fabs (fsize - size);

		    if (distance <= min_distance || min_distance < 0)
		    {
			    min_distance = distance;
			    best = i;
		    }
	    }
#if HAVE_FT_BITMAP_SIZE_Y_PPEM
	    error = FT_Set_Char_Size (face,
				      face->available_sizes[best].x_ppem,
				      face->available_sizes[best].y_ppem,
				      0, 0);
	    if (error)
#endif
		    error = FT_Set_Pixel_Sizes (face,
						face->available_sizes[best].width,
						face->available_sizes[best].height);
    }
}

void FreeTypeBaseFont::UnlockFace()
{
	mLockCnt--;
	mCritSect.Leave();
}

bool FreeTypeBaseFont::IsFaceLocked()
{
	AutoCrit anAutoCrit(mCritSect);

	return mLockCnt != 0;
}

bool FreeTypeBaseFont::HasUnlockedFace()
{
	AutoCrit anAutoCrit(mCritSect);

	return !mLockCnt && mFace;
}

bool FreeTypeBaseFont::DropUnlockedFace()
{
	AutoCrit anAutoCrit(mCritSect);

	if (!mLockCnt && mFace)
	{
		FT_Done_Face (mFace);
		mFace = 0;
		mHasSize = false;

		return true;
	}

	return false;
}
