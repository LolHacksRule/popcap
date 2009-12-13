#ifndef __IMAGELIB_H__
#define __IMAGELIB_H__

#include <string>


 #if defined(WIN32) && !defined(BUILDING_STATIC_IMAGELIB)
#ifdef BUILDING_IMAGELIB
#define IMAGELIB_EXPORT __declspec (dllexport)
#else
#define IMAGELIB_EXPORT __declspec (dllimport)
#endif
#endif

#ifndef IMAGELIB_EXPORT
#define IMAGELIB_EXPORT
#endif

namespace ImageLib
{

class IMAGELIB_EXPORT Image
{
public:
	int						mWidth;
	int						mHeight;
	unsigned int *			mBits;

public:
	Image();
	virtual ~Image();

	int						GetWidth();
	int						GetHeight();
	unsigned int*			GetBits();
};

IMAGELIB_EXPORT bool WriteJPEGImage(const std::string& theFileName, Image* theImage);
IMAGELIB_EXPORT bool WritePNGImage(const std::string& theFileName, Image* theImage);
IMAGELIB_EXPORT bool WriteTGAImage(const std::string& theFileName, Image* theImage);
IMAGELIB_EXPORT bool WriteBMPImage(const std::string& theFileName, Image* theImage);
extern IMAGELIB_EXPORT int gAlphaComposeColor;
extern IMAGELIB_EXPORT bool gAutoLoadAlpha;
extern IMAGELIB_EXPORT bool gIgnoreJPEG2000Alpha;  // I've noticed alpha in jpeg2000's that shouldn't have alpha so this defaults to true


IMAGELIB_EXPORT Image* GetImage(const std::string& theFileName, bool lookForAlphaImage = true);

IMAGELIB_EXPORT void InitJPEG2000();
IMAGELIB_EXPORT void CloseJPEG2000();
IMAGELIB_EXPORT void SetJ2KCodecKey(const std::string& theKey);

}

#endif //__IMAGELIB_H__
