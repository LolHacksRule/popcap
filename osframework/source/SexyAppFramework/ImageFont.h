#ifndef __IMAGEFONT_H__
#define __IMAGEFONT_H__

#include "Font.h"
#include "DescParser.h"
#include "SharedImage.h"

namespace Sexy
{

class SexyAppBase;
class Image;

typedef std::map<int, int> SexyCharToIntMap;
class CharData
{
public:
	Rect					mImageRect;
	Point					mOffset;
	SexyCharToIntMap			mKerningOffsets;
	short					mWidth;
	short					mOrder;

public:
	CharData();
};

class FontData;

typedef std::map<int, CharData> CharDataMap;
class FontLayer
{
public:
	typedef std::vector<std::string> StringVector;
	typedef std::map<std::wstring, std::wstring> ExInfoMap;
public:
	FontData*				mFontData;
	ExInfoMap				mExtendedInfo;
	WStringVector			mRequiredTags;
	WStringVector			mExcludedTags;
	CharDataMap				mCharDataMap;
	Color					mColorMult;
	Color					mColorAdd;
	SharedImageRef			mImage;
	int						mDrawMode;
	Point					mOffset;
	int						mSpacing;
	int						mMinPointSize;
	int						mMaxPointSize;
	int						mPointSize;
	int						mAscent;
	int						mAscentPadding; // How much space is above the avg uppercase char
	int						mHeight;		//
	int						mDefaultHeight; // Max height of font character image rects
	int						mLineSpacingOffset; // This plus height should get added between lines
	int						mBaseOrder;

public:
	FontLayer(FontData* theFontData);
	FontLayer(const FontLayer& theFontLayer);

	CharData*				SetCharData(int theChar);
	CharData*				GetCharData(int theChar);

	bool					isGlyph(int theChar);
};

typedef std::list<FontLayer> FontLayerList;
typedef std::map<std::wstring, FontLayer*> FontLayerMap;
typedef std::list<Rect> RectList;
typedef std::vector<int> IntVector;

typedef std::map<int, int> SexyCharToSexyCharMap;

class FontData : public DescParser
{
public:
	bool					mInitialized;
	int						mRefCount;
	SexyAppBase*			mApp;

	int						mDefaultPointSize;
	SexyCharToSexyCharMap		mCharMap;
	FontLayerList			mFontLayerList;
	FontLayerMap			mFontLayerMap;

	std::string				mSourceFile;
	std::string				mFontErrorHeader;

public:
	virtual bool			Error(const std::string& theError);

	bool					GetColorFromDataElement(DataElement *theElement, Color &theColor);
	bool					DataToLayer(DataElement* theSource, FontLayer** theFontLayer);
	virtual bool			HandleCommand(const ListDataElement& theParams);

public:
	FontData();
	virtual ~FontData();

	void					Ref();
	void					DeRef();

	bool					Load(SexyAppBase* theSexyApp, const std::string& theFontDescFileName);
	bool					LoadLegacy(Image* theFontImage, const std::string& theFontDescFileName);
};

typedef std::map<int, Rect> SexyCharToRectMap;
class ActiveFontLayer
{
public:
	FontLayer*				mBaseFontLayer;

	Image*					mScaledImage;
	bool					mOwnsImage;
	SexyCharToRectMap			mScaledCharImageRects;

public:
	ActiveFontLayer();
	ActiveFontLayer(const ActiveFontLayer& theActiveFontLayer);
	virtual ~ActiveFontLayer();
};

typedef std::list<ActiveFontLayer> ActiveFontLayerList;

class RenderCommand
{
public:
	Image*					mImage;
	int						mDest[2];
	int						mSrc[4];
	int						mMode;
	Color					mColor;
	RenderCommand*			mNext;
};

typedef std::multimap<int, RenderCommand> RenderCommandMap;

class ImageFont : public Font
{
public:
	typedef std::vector<std::string> StringVector;
public:
	FontData*				mFontData;
	int						mPointSize;
	WStringVector			mTagVector;

	bool					mActiveListValid;
	ActiveFontLayerList		mActiveLayerList;
	double					mScale;
	bool					mForceScaledImagesWhite;

public:
	virtual void			GenerateActiveFontLayers();
	virtual void			DrawStringEx(Graphics* g, int theX, int theY,
						     const std::string& theString,
						     const Color& theColor, const Rect* theClipRect,
						     RectList* theDrawnAreas, int* theWidth);
	virtual void			DrawStringEx(Graphics* g, int theX, int theY,
						     const std::wstring& theString,
						     const Color& theColor, const Rect* theClipRect,
						     RectList* theDrawnAreas, int* theWidth);
public:
	ImageFont(SexyAppBase* theSexyApp, const std::string& theFontDescFileName);
	ImageFont(Image *theFontImage); // for constructing your own image font without a file descriptor
	ImageFont(const ImageFont& theImageFont);
	virtual ~ImageFont();

	// Deprecated
	ImageFont(Image* theFontImage, const std::string& theFontDescFileName);
	//ImageFont(const ImageFont& theImageFont, Image* theImage);

	virtual int			CharWidth(int theChar);
	virtual int			CharWidthKern(int theChar, int thePrevChar);
	virtual int			StringWidth(const std::string& theString);
	virtual void			DrawString(Graphics* g, int theX, int theY,
						   const std::string& theString,
						   const Color& theColor,
						   const Rect& theClipRect);

	virtual int			StringWidth(const std::wstring& theString);
	virtual void			DrawString(Graphics* g, int theX, int theY,
						   const std::wstring& theString,
						   const Color& theColor,
						   const Rect& theClipRect);

	virtual Font*			Duplicate();

	virtual void			SetPointSize(int thePointSize);
	virtual int				GetPointSize();
	virtual void			SetScale(double theScale);
	virtual int				GetDefaultPointSize();
	virtual bool			AddTag(const std::wstring& theTagName);
	virtual bool			RemoveTag(const std::wstring& theTagName);
	virtual bool			HasTag(const std::wstring& theTagName);
	virtual bool			AddTag(const std::string& theTagName);
	virtual bool			RemoveTag(const std::string& theTagName);
	virtual bool			HasTag(const std::string& theTagName);
	virtual std::wstring		GetDefine(const std::wstring& theName);

	virtual void			Prepare();
	virtual  bool                   StringToGlyphs(const std::wstring &theString,
						       GlyphVector &theGlyphs);
	virtual  void                   DrawGlyphs(Graphics *g, int theX, int theY,
						   GlyphVector& theGlyphs, const Color &theColor,
						   const Rect& theClipRect);

	int                             GetMappedChar(int theChar);
};

}

#endif //__IMAGEFONT_H__
