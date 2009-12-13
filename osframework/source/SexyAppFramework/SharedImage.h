#ifndef __SHARED_IMAGE_H__
#define __SHARED_IMAGE_H__

#include "Common.h"

namespace Sexy
{

class Image;
class DDImage;
class MemoryImage;

class SEXY_EXPORT SharedImage
{
public:
	Image*				mImage;
	int						mRefCount;

	SharedImage();
};

typedef std::map<std::pair<std::string, std::string>, SharedImage> SharedImageMap;

class SEXY_EXPORT SharedImageRef
{
public:
	SharedImage*			mSharedImage;
	MemoryImage*			mUnsharedImage;
	bool					mOwnsUnshared;

public:
	SharedImageRef();
	SharedImageRef(const SharedImageRef& theSharedImageRef);
	SharedImageRef(SharedImage* theSharedImage);
	~SharedImageRef();

	void					Release();

	SharedImageRef&			operator=(const SharedImageRef& theSharedImageRef);
	SharedImageRef&			operator=(SharedImage* theSharedImage);
	SharedImageRef&			operator=(MemoryImage* theUnsharedImage);
	MemoryImage*			operator->();
	operator Image*();
	operator MemoryImage*();
};

}

#endif //__SHARED_IMAGE_H__
